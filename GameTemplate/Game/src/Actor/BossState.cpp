#include "stdafx.h"
#include "BossState.h"
#include "BossCharacter.h"
#include "AttackRange.h"
#include "src/Actor/ActorStatus.h"
#include "src/Actor/AttackObjectManager.h"
#include "src/Stage/GrassBendManager.h"
#include <time.h>

// ============================================================================
// 調整値について
// ----------------------------------------------------------------------------
// 距離・秒数・コリジョンサイズ・エフェクトスケールなどのバランス調整値は
// Assets/Parameter/BossStateParameter.json (MasterBossStateParameter) に外出しした。
// 各ステートの Enter()/Update() 内で ParameterManager::Get().GetBossStateParam() から
// 都度読み出して使う（"p->通常攻撃.collisionSize" のように attack 名 + 分類でまとめてある）。
// ============================================================================
namespace
{
	// 以下は「バランス調整値」ではなく、コード上の意味を分かりやすくするための
	// 数値定数（0・1・0.5など）。JSON化する対象ではないためここに残している。
	constexpr uint8_t INT_ZERO = 0;				// 初期化の値(int)
	constexpr float FLOAT_ZERO = 0.0f;			// 初期化の値(float)
	constexpr float BOSS_ROTATE_MAX = 360.0f;	// 1回転
	constexpr float MOVE_EPSILON = 0.001f;		// 少しでも動いているか
	constexpr float DIST_HALF = 0.5f;			// 距離の半分
	constexpr float FIXED_EFFECT_SCALE_Y = 1.0f;// エフェクトのｙスケールの初期化
}


/*==========================================*/
// 待機の状態
/*==========================================*/

void BossIdleState::Enter() 
{
	isFinished_ = false;
	boss_->PlayAnimation(BossAnimID::enAnimIdle);

	// 待機時間の生成
	{
		const auto* p = ParameterManager::Get().GetBossStateParam();

		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// 時間設定（Idleは「攻撃」ではないので攻撃速度の影響は受けない）
		taskScheduler_->AddTimer(p->idle.endTime, [&]() {
			isFinished_ = true;
			});
	}
}

void BossIdleState::Update()
{
	boss_->SetMoveVelocity(Vector3::Zero); // 移動速度を0に

	if (taskScheduler_ && BattleManager::Get().IsPlayingScene()) { 
		taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); 
	}
}

void BossIdleState::Exit()
{
	taskScheduler_.reset();
}


/*==========================================*/
// 走る状態
/*==========================================*/

void BossRunState::Enter()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();

	isFinished_ = false; // 初期化
	boss_->PlayAnimation(BossAnimID::enAnimRun); // 走るアニメーションを設定

	// プレイヤーとの距離を求める
	Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
	diff.y = FLOAT_ZERO;
	float currentDistSq = diff.LengthSq(); // 今の距離の2乗

	// 射程範囲外から走り始めた → 遠距離の境界まで走る
	if (currentDistSq > p->common.longDistance * p->common.longDistance) {
		goalPos_ = p->common.longDistance * p->common.longDistance;
	}
	// 遠距離から走り始めた → 中距離の境界まで走る
	else if (currentDistSq > p->common.midDistance * p->common.midDistance) {
		goalPos_ = p->common.midDistance * p->common.midDistance;
	}
	// 中距離から走り始めた → 近距離の境界まで走る
	else if (currentDistSq > p->common.shortDistance * p->common.shortDistance) {
		goalPos_ = p->common.shortDistance * p->common.shortDistance;
	}

	// 音の再生（Runは「攻撃」ではないので攻撃速度の影響は受けない）
	{
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		const int SoundId = taskScheduler_->CreateLoopSequence(p->run.seLoopInterval);
		taskScheduler_->AddLoopTimer(SoundId, FLOAT_ZERO, [&](void) {
			SoundManager::Get().PlaySE(enSoundKind_Boss_Run);
			});
	}

	// エフェクト
	{
		const int SoundId = taskScheduler_->CreateLoopSequence(p->run.effectLoopInterval);
		taskScheduler_->AddLoopTimer(SoundId, FLOAT_ZERO, [&](void) {
			const Vector3 bossPos = boss_->transform_.position;
			const Quaternion bossRot = boss_->transform_.rotation;
			EffectManager::Get().PlayEffect(enEffectKind_Dash_Wind, bossPos, bossRot, ParameterManager::Get().GetBossStateParam()->run.effectScale);
			});
	}
}

void BossRunState::Update()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();

	// ステートを抜けるかどうか
	{
		Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
		diff.y = FLOAT_ZERO;

		if (diff.LengthSq() <= goalPos_)
		{
			isFinished_ = true;
			boss_->SetMoveVelocity(Vector3::Zero); // ピタッと止まる
			boss_->PlayAnimation(BossAnimID::enAnimIdle); // 待機アニメーション開始
			return;
		}
	}


	// 移動 / 回転の計算・設定
	{
		Vector3 moveVelocity = CalcMovePlayerVelocity(p->run.moveSpeed);
		Quaternion moveRotate = RotateToTarget(p->common.rotateSpeed);
		boss_->SetMoveVelocity(moveVelocity);
		boss_->SetTargetRot(moveRotate);
	}

	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void BossRunState::Exit()
{
	taskScheduler_.reset();
}


/*==========================================*/
// 通常攻撃状態
/*==========================================*/

void BossAttackState::Enter()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float speedMul = GetAttackSpeedMul(); // 攻撃速度倍率（[timing]の秒数はこれで割る）

	isFinished_ = false; // 初期化
	Quaternion targetRot = RotateToTarget(p->common.rotateSpeed);// 攻撃の前にプレイヤーがいる方向へ向く
	boss_->SetTargetRot(targetRot);

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 攻撃までの時間設定
	{
		// アニメーションの再生とコリジョンの生成
		taskScheduler_->AddTimer(p->normalAttack.beginTime / speedMul, [&]() {
			// タイマー発火時点のパラメーターを取り直す（Enter()のローカル変数pは寿命が切れているため使えない）
			const auto* p = ParameterManager::Get().GetBossStateParam();
			const float animSpeed = GetAnimationSpeedMul(); // アニメーション再生速度（攻撃速度とは独立）
			const float effectSpeed = GetEffectSpeedMul();  // エフェクト再生速度（攻撃速度とは独立）

			boss_->PlayAnimation(BossAnimID::enAnimAttack, animSpeed); // 通常攻撃アニメーションを設定
			SoundManager::Get().PlaySE(enSoundKind_Boss_NormalAttack); // 音の再生

			// ゴーストコリジョンを生成
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossNormalAtkID(), p->normalAttack.collisionSize, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);

			// 座標計算
			Vector3 bossPos = boss_->transform_.position;				// プレイヤーの現在の座標を取得

			const Matrix& mat = boss_->transform_.GetWorldMatrix(); // ボスのワールド座標を取得
			Vector3 forward(mat.m[2][0], mat.m[2][1], mat.m[2][2]); // Z軸
			forward.Normalize(); // ベクトルの長さを1にしておく

			Quaternion bossRot = boss_->GetTargetRot();
			float forwardOffset = p->normalAttack.collisionForward;									// 目の前にどれくらいズラすか
			float heightOffset = p->normalAttack.collisionHeight;										// 高さの調整
			Vector3 collisionTargetPos = bossPos + (forward * forwardOffset);	// 前方向の座標を決定
			collisionTargetPos.y += heightOffset;									// 高さを決定

			// コリジョンの座標を設定
			attackHitbox_->SetPosition(collisionTargetPos);

			// 草を曲げる
			if (GrassBendManager::IsInitialized())
			{
				if (const auto* gp = ParameterManager::Get().GetGrassBendParam("NormalAttack"))
				{
					GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
					GrassBendManager::Get().AddSource(collisionTargetPos, params);
				}
			}

			// エフェクトのPRSを決め
			Vector3 collisionTargetScal = p->normalAttack.collisionSize * p->common.damageRingEffectScale;
			bossRot.AddRotationDegY(BOSS_ROTATE_MAX);
			EffectManager::Get().PlayEffect(enEffectKind_Wind_Blast_Boss, collisionTargetPos, bossRot, collisionTargetScal, effectSpeed);
			});

		// コリジョンを破棄
		taskScheduler_->AddTimer(p->normalAttack.collisionResetTime / speedMul, [&]()
			{
				attackHitbox_.reset(nullptr);
			},
			true);

		// 処理を終わる
		taskScheduler_->AddTimer(p->normalAttack.endTime / speedMul, [&]()
			{
				isFinished_ = true;
			});
	}
}

void BossAttackState::Update()
{
	boss_->SetMoveVelocity(Vector3::Zero); // 移動速度を0に

	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void BossAttackState::Exit()
{
	taskScheduler_.reset();
}


/*==========================================*/
// ヒットスタンプ
/*==========================================*/

void HitStampState::Enter()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float speedMul = GetAttackSpeedMul(); // 攻撃速度倍率（[timing]の秒数はこれで割る）

	isFinished_ = false; // 初期化
	createAttackCollision_ = false; // コリジョンの生成を可能にする
	gravity_ = p->hitStamp.gravityPower;
	phase_ = Phase::Ready; // 準備をする

	Quaternion targetRot = RotateToTarget(p->common.rotateSpeed);// 攻撃の前にプレイヤーがいる方向へ向く
	boss_->SetTargetRot(targetRot);

	boss_->PlayAnimation(BossAnimID::enAnimJump, GetAnimationSpeedMul()); // ジャンプアニメーションを設定

	// 攻撃範囲インジケーターを生成 (表示は Hover フェーズまで待つ)
	{
		attackRange_ = NewGO<AttackRange>(0, "hitStampRange");
		AttackRange::InitParam param;
		param.type      = AttackRange::Type::enCircle;
		param.character = AttackRange::Character::Boss;
		attackRange_->SetInitParam(param);
		attackRange_->SetScale(Vector3(p->hitStamp.rangeSize, 1.0f, p->hitStamp.rangeSize));
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 現在の位置から上方向に飛び上がる
	taskScheduler_->AddTimer(p->hitStamp.upBeginTime / speedMul, [&]() {
		const auto* p = ParameterManager::Get().GetBossStateParam(); // タイマー発火時点で取り直す

		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = boss_->GetTransformPosition() + p->hitStamp.jumpHeight; // 移動する地点を設定
		nextTargetPos_ = boss_->GetTargetPos() + p->hitStamp.jumpHeight;// 次に移動する地点を計算
		verticalVelocity_ = p->hitStamp.verticalVelocity; // 垂直速度
		phase_ = Phase::JumpUp;
		});

	// プレイヤーの頭上へ移動
	taskScheduler_->AddTimer(p->hitStamp.overheadMoveTime / speedMul, [&]() {
		const auto* p = ParameterManager::Get().GetBossStateParam(); // タイマー発火時点で取り直す

		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = nextTargetPos_;// 移動する地点を設定

		// このタイミングでプレイヤー座標を確定させ、以降は追従しない
		Vector3 playerPos = boss_->GetTargetPos();
		fixedAttackPos_ = Vector3(playerPos.x, 0.1f, playerPos.z); // 落下地点（地面）を固定

		// 攻撃範囲インジケーターを固定地点に表示 (少し浮かせて地面とのZファイトを防ぐ)
		if (attackRange_)
		{
			attackRange_->SetPosition(Vector3(fixedAttackPos_.x, 0.5f, fixedAttackPos_.z));
			// カウントダウン半分の時点で外円到達し、残り半分は外円に張り付いて表示
			const float speedMul = GetAttackSpeedMul();
			const float countdown = (p->hitStamp.fallBeginTime - p->hitStamp.overheadMoveTime) / speedMul;
			attackRange_->SetPulseCountdown(countdown * 0.5f);
			attackRange_->SetDraw(true);
		}
		phase_ = Phase::Hover;
		});

	// 地面に落ちる
	taskScheduler_->AddTimer(p->hitStamp.fallBeginTime / speedMul, [&]() {

		boss_->SetMoveVelocity(Vector3::Zero);
		verticalVelocity_ = FLOAT_ZERO;

		// Hover 時に確定した固定地点へ落下（プレイヤーが動いても追わない）
		targetPos_ = fixedAttackPos_;

		// 落下開始と同時にインジケーターを非表示にする
		if (attackRange_)
		{
			attackRange_->SetDraw(false);
		}
		phase_ = Phase::FallDown;
		});
}

void HitStampState::Update()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float effectSpeed = GetEffectSpeedMul(); // 着地エフェクトの再生速度（攻撃速度とは独立）

	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	switch (phase_)
	{
	case Phase::Ready:
	{
		boss_->SetMoveVelocity(Vector3::Zero);
		break;
	}

	case Phase::JumpUp:
	{
		// XZ移動
		Vector3 move = CalcVelocityTowards(targetPos_, p->hitStamp.upSpeed);
		move.y = FLOAT_ZERO;
		boss_->SetMoveVelocity(move);

		// Y軸は重力で管理
		verticalVelocity_ += gravity_ * g_gameTime->GetFrameDeltaTime();
		boss_->transform_.localPosition.y += verticalVelocity_ * g_gameTime->GetFrameDeltaTime();

		// 目標高度を超えたらHoverへ
		if (boss_->GetTransformPosition().y >= targetPos_.y)
		{
			verticalVelocity_ = FLOAT_ZERO;
			phase_ = Phase::Hover;
		}
		break;
	}

	case Phase::Hover:
	{
		// Hover突入時に確定した攻撃地点の真上に貼り付く（プレイヤーは追わない）
		Vector3 currentBossPos = boss_->GetTransformPosition();
		targetPos_ = Vector3(fixedAttackPos_.x, currentBossPos.y, fixedAttackPos_.z);
		boss_->transform_.localPosition = targetPos_;
		boss_->transform_.UpdateTransform();
		// エフェクトは固定座標に置いたままなので毎フレーム更新不要
		break;
	}

	case Phase::FallDown:
	{
		Vector3 move = CalcVelocityTowards(targetPos_, p->hitStamp.downSpeed);
		move.y = FLOAT_ZERO;
		boss_->SetMoveVelocity(move);

		verticalVelocity_ += gravity_ * g_gameTime->GetFrameDeltaTime();
		boss_->transform_.localPosition.y += verticalVelocity_ * g_gameTime->GetFrameDeltaTime();

		if (boss_->GetTransformPosition().y <= FLOAT_ZERO)
		{
			boss_->transform_.localPosition.y = FLOAT_ZERO;
			verticalVelocity_ = FLOAT_ZERO;

			// ここに着地時の処理をまとめる
			boss_->SetMoveVelocity(Vector3::Zero); // 移動速度を0に
			Vector3 targetPos = boss_->GetTransformPosition(); // ボスの座標を取得
			// ※実際の攻撃判定半径はコード内の固定値(300.0f)を使用しており、effectScaleBasisとは別（詳細はShokingStampフェーズ参照）
			Vector3 targetSmokeScal = p->hitStamp.effectScaleBasis * p->hitStamp.smokeEffectScale;	// 煙のエフェクトのスケール
			Vector3 targetShokeWaveScal = p->hitStamp.effectScaleBasis * p->hitStamp.shockWaveEffectScale; // 衝撃波のエフェクトのスケール
			EffectManager::Get().PlayEffect(enEffectKind_HitStamp_Smoke, targetPos, boss_->GetTransformRotation(), targetSmokeScal, effectSpeed); // 煙
			EffectManager::Get().PlayEffect(enEffectKind_HitStamp_ShockWave, targetPos, boss_->GetTransformRotation(), targetShokeWaveScal, effectSpeed); // 衝撃波
			SoundManager::Get().PlaySE(enSoundKind_Boss_HitStamp);

			phase_ = Phase::ShokingStamp;
		}
		break;
	}

	case Phase::ShokingStamp:
	{
		// 作られてないならゴーストコリジョンを生成、作られているなら最終フェーズへ
		if (createAttackCollision_) {
			phase_ = Phase::Finished;
		}
		else {
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossHitStampAtkID(), 300.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			// 固定した攻撃地点に配置（ボスの着地座標 = fixedAttackPos_）
			attackHitbox_->SetPosition(fixedAttackPos_);
			boss_->SetMoveVelocity(Vector3::Zero);
			createAttackCollision_ = true;

			// 草を曲げる
			if (GrassBendManager::IsInitialized())
			{
				if (const auto* gp = ParameterManager::Get().GetGrassBendParam("HitStamp"))
				{
					GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
					GrassBendManager::Get().AddSource(fixedAttackPos_, params);
				}
			}
		}
		break;
	}

	case Phase::Finished:
	{
		attackHitbox_.reset();
		isFinished_ = true;
		break;
	}
	}
}

void HitStampState::Exit()
{
	// インジケーターが残っていたら確実に破棄する（FallDown以降で非表示になるが保険として）
	if (attackRange_)
	{
		attackRange_->SetDraw(false);
		DeleteGO(attackRange_);
		attackRange_ = nullptr;
	}
	taskScheduler_.reset(nullptr);
}


/*==========================================*/
// 回転攻撃
/*==========================================*/

void SpinState::Enter()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float speedMul = GetAttackSpeedMul(); // 攻撃速度倍率（[timing]の秒数はこれで割る）

	isFinished_ = false; // 初期化
	isAttackStart_ = false; // 攻撃範囲表示の間は攻撃を開始しない

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(p->common.rotateSpeed);
	boss_->SetTargetRot(targetRot);

	// 回転アニメーションを設定
	boss_->PlayAnimation(BossAnimID::enAnimSpin, GetAnimationSpeedMul());

	// エフェクトを再生
	Vector3 targetScal = p->spin.effectScaleBasis * p->common.damageRingEffectScale;
	spinEffectHandle_ = EffectManager::Get().PlayEffect(
		enEffectkind_Spin,
		boss_->GetTransformPosition(),
		boss_->GetTransformRotation(),
		targetScal,
		GetEffectSpeedMul()
	);

	// プレイヤーの方向と進む距離を設定
	{
		Vector3 playerPos = boss_->GetTargetPos();			// プレイヤーの座標
		Vector3 bossPos = boss_->GetTransformPosition();	// ボスの座標

		Vector3 diff = playerPos - bossPos; // 方向を算出
		if (diff.LengthSq() >= MOVE_EPSILON)
		{
			diff.Normalize(); // 方向を正規化
			targetPos_ = playerPos + (diff * p->spin.overMoveDistance); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
		}
		else {
			targetPos_ = playerPos;
		}
	}

	// ボスから targetPos_ まで AttackRange (Line) で攻撃予測ラインを表示
	{
		predictionElapsed_ = 0.0f;

		Vector3 bossPos = boss_->GetTransformPosition();
		Vector3 dir     = targetPos_ - bossPos;
		float totalDist = dir.Length();
		if (totalDist >= MOVE_EPSILON)
		{
			dir.y = 0.0f;
			dir.Normalize();

			// ライン中心 = ボスから indicatorLength の半分先
			Vector3 startPos = bossPos + (dir * (p->spin.indicatorLength * DIST_HALF + p->spin.indicatorForward));
			startPos.y = 0.5f;

			// 方向に合わせてY軸回転
			float angle = atan2(dir.x, dir.z);
			Quaternion lineRot;
			lineRot.SetRotationY(angle);

			// AttackRange 生成
			attackRangeIndicator_ = NewGO<AttackRange>(0, "spinRange");
			AttackRange::InitParam param;
			param.type             = AttackRange::Type::enLine;
			param.character        = AttackRange::Character::Boss;
			param.baseSizeX        = 200.0f; // 200単位ごとにタイリング (長さ軸は X)
			attackRangeIndicator_->SetInitParam(param);
			attackRangeIndicator_->SetPosition(startPos);
			attackRangeIndicator_->SetRotation(lineRot);
			attackRangeIndicator_->SetScale(Vector3(p->spin.indicatorLength * 0.5f, FIXED_EFFECT_SCALE_Y, p->spin.indicatorRangeSize));
			attackRangeIndicator_->SetLineDrawProgress(0.0f); // 初期は非表示
			attackRangeIndicator_->SetDraw(true);
		}
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// タスクシステムの構築
	{
		// 移動開始（予測インジケーターを非表示にしてから攻撃）
		taskScheduler_->AddTimer(p->spin.attackStartTime / speedMul, [&]() {
			const auto* p = ParameterManager::Get().GetBossStateParam(); // タイマー発火時点で取り直す

			// 予測インジケーターを非表示・破棄
			if (attackRangeIndicator_)
			{
				attackRangeIndicator_->SetDraw(false);
				DeleteGO(attackRangeIndicator_);
				attackRangeIndicator_ = nullptr;
			}

			// コリジョンの生成
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossSpinAtkID(), 200.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			isAttackStart_ = true;
			// 草曲げは Update() で毎フレーム SetSource を呼ぶ

			// SEの再生
			const int id = taskScheduler_->CreateLoopSequence(p->spin.seLoopInterval);
			taskScheduler_->AddLoopTimer(id, FLOAT_ZERO, [&]() {
				SoundManager::Get().PlaySE(enSoundKind_Boss_Spin);
				});
			});


		// 強制終了
		taskScheduler_->AddTimer(p->spin.attackEndTime / speedMul, [&]() {
			boss_->SetMoveVelocity(Vector3::Zero);
			attackHitbox_.reset();
			isFinished_ = true;
			});
	}

}

void SpinState::Update()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float speedMul = GetAttackSpeedMul();

	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	// 攻撃開始前はラインインジケーターをボスから徐々に伸ばす
	if (!isAttackStart_ && attackRangeIndicator_)
	{
		predictionElapsed_ += g_gameTime->GetFrameDeltaTime();
		float progress = min(predictionElapsed_ / (p->spin.attackStartTime / speedMul), 1.0f);
		attackRangeIndicator_->SetLineDrawProgress(progress);
	}

	// エフェクトをボスの足元に追従させる
	if (spinEffectHandle_ != INVALID_EFFECT_HANDLE)
	{
		EffectManager::Get().SetEffectPosition(spinEffectHandle_, boss_->GetTransformPosition());
		EffectManager::Get().SetEffectRotation(spinEffectHandle_, boss_->GetTransformRotation());
	}

	if (isAttackStart_)
	{
		// 移動先を計算・設定
		Vector3 moveVelocity = CalcVelocityTowards(targetPos_, p->spin.moveSpeed);
		boss_->SetMoveVelocity(moveVelocity);

		if (attackHitbox_) { attackHitbox_->SetPosition(boss_->GetTransformPosition()); }

		// ボスの現在位置で草を曲げ続ける (elapsed をリセットすることで攻撃中は常に全力維持)
		if (GrassBendManager::IsInitialized())
		{
			if (const auto* gp = ParameterManager::Get().GetGrassBendParam("SpinAttack"))
			{
				GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
				GrassBendManager::Get().SetSource(GrassBendManager::SPIN_ATTACK_SLOT, boss_->GetTransformPosition(), params);
			}
		}

		if (moveVelocity.LengthSq() < MOVE_EPSILON)
		{
			boss_->SetMoveVelocity(Vector3::Zero);	// ピタッと止める
			attackHitbox_.reset();					// 当たり判定の破棄
			isFinished_ = true;						// ステート終了！
		}
	}
}

void SpinState::Exit()
{
	// 追従エフェクトを停止・後始末
	if (spinEffectHandle_ != INVALID_EFFECT_HANDLE)
	{
		EffectManager::Get().StopEffect(spinEffectHandle_);
		spinEffectHandle_ = INVALID_EFFECT_HANDLE;
	}

	// 予測インジケーターが残っていたら破棄（タイマー起動前に強制終了した場合の保険）
	if (attackRangeIndicator_)
	{
		DeleteGO(attackRangeIndicator_);
		attackRangeIndicator_ = nullptr;
	}

	taskScheduler_.reset(nullptr);
}


/*==========================================*/
// 岩を投げつける攻撃
/*==========================================*/

void ThrowRockState::Enter()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float speedMul = GetAttackSpeedMul(); // 攻撃速度倍率（[timing]の秒数はこれで割る）

	isFinished_ = false; // 初期化
	boss_->SetMoveVelocity(Vector3::Zero);

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(p->common.rotateSpeed);
	boss_->SetTargetRot(targetRot);


	// プレイヤーとボスの現在の座標を取得
	boss_->transform_.UpdateWorldMatrix();
	Vector3 playerPos = boss_->GetTargetPos();
	Vector3 bossPos = boss_->GetTransformPosition();


	// 方向を算出して目標の座標を設定
	Vector3 diff = playerPos - bossPos;
	if (diff.LengthSq() >= MOVE_EPSILON)
	{
		diff.Normalize(); // 方向を正規化
		targetPos_ = playerPos + (diff * p->throwRock.overMoveDistance); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
	}
	else {
		targetPos_ = playerPos;
	}

	// 攻撃予測ラインインジケーターの設定 (SpinState と同じパターン)
	predictionElapsed_ = 0.0f;
	Vector3 targetDir = targetPos_ - bossPos;
	float totalDist = targetDir.Length();

	if (totalDist >= MOVE_EPSILON)
	{
		targetDir.Normalize();

		float meshHalfExtent = p->throwRock.indicatorLength * DIST_HALF;
		Vector3 startPos = bossPos + targetDir * (p->throwRock.indicatorLength * DIST_HALF + p->throwRock.indicatorForward);
		startPos.y = 0.5f;

		float angle = atan2(targetDir.x, targetDir.z);
		Quaternion lineRot;
		lineRot.SetRotationY(angle);

		attackRangeIndicator_ = NewGO<AttackRange>(0, "rockRange");
		AttackRange::InitParam param;
		param.type      = AttackRange::Type::enLine;
		param.character = AttackRange::Character::Boss;
		param.baseSizeX = p->throwRock.indicatorBaseSize;
		attackRangeIndicator_->SetInitParam(param);
		attackRangeIndicator_->SetPosition(startPos);
		attackRangeIndicator_->SetRotation(lineRot);
		attackRangeIndicator_->SetScale(Vector3(meshHalfExtent, FIXED_EFFECT_SCALE_Y, p->throwRock.indicatorRangeSize));
		attackRangeIndicator_->SetLineDrawProgress(0.0f);
		attackRangeIndicator_->SetDraw(true);
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	taskScheduler_->AddTimer(p->throwRock.beginTime / speedMul, [this,bossPos,targetDir]()
		{
			// タイマー発火時点で取り直す（Enter()のローカル変数pは寿命が切れているため使えない）
			const auto* p = ParameterManager::Get().GetBossStateParam();

			// 予測インジケーターを非表示・破棄
			if (attackRangeIndicator_)
			{
				attackRangeIndicator_->SetDraw(false);
				DeleteGO(attackRangeIndicator_);
				attackRangeIndicator_ = nullptr;
			}

			// 岩の初期位置の計算
			const Matrix& mat = boss_->transform_.GetWorldMatrix(); // ボスのワールド座標を取得
			Vector3 forward(mat.m[2][0], mat.m[2][1], mat.m[2][2]); // Z軸
			forward.Normalize(); // ベクトルの長さを1にしておく

			// ※通常攻撃(normalAttack)と同じ前方/高さオフセット値を使い回している（元コードの仕様を踏襲）
			float attackForward = p->normalAttack.collisionForward;
			float attackHeight = p->normalAttack.collisionHeight;
			Vector3 startPos = bossPos + (forward * attackForward);
			startPos.y = attackHeight;

			// 岩の生成
			AttackObjectManager::Get().CreateRock(
				boss_,
				startPos,
				targetDir,
				p->throwRock.rockCollisionSize
			);
			// アニメーションを再生
			boss_->PlayAnimation(BossAnimID::enAnimClicked, GetAnimationSpeedMul());
			SoundManager::Get().PlaySE(enSoundKind_Boss_ThrowAttack); // 音の再生
		});

	taskScheduler_->AddTimer(p->throwRock.endTime / speedMul, [&]()
		{
			isFinished_ = true;
		});
}

void ThrowRockState::Update()
{
	const auto* p = ParameterManager::Get().GetBossStateParam();
	const float speedMul = GetAttackSpeedMul();

	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	if (attackRangeIndicator_)
	{
		predictionElapsed_ += g_gameTime->GetFrameDeltaTime();
		float progress = min(predictionElapsed_ / (p->throwRock.beginTime / speedMul), 1.0f);
		attackRangeIndicator_->SetLineDrawProgress(progress);
	}
}

void ThrowRockState::Exit()
{
	if (attackRangeIndicator_)
	{
		DeleteGO(attackRangeIndicator_);
		attackRangeIndicator_ = nullptr;
	}

	taskScheduler_.reset();
}


/*==========================================*/
// レーザー攻撃
/*==========================================*/

void LaserState::Setup()
{
	// Enter()から呼ばれる（同一フレーム内の同期呼び出しのため、ここでパラメーターを取得してよい）
	const auto* p = ParameterManager::Get().GetBossStateParam();

	shotTime_ = p->laser.initialShotTime / attackSpeedMul_;
	attackDeleyTime = p->laser.initialShotTime / attackSpeedMul_;

	switch (mode_)
	{
	case Mode::enNormal:
	{
		scale_ = Vector3::One;
		attackDeleyTime = p->laser.shotIntervalNormal / attackSpeedMul_;
		shotCount_ = p->laser.shotCountNormal;
		break;
	}
	case Mode::enMult:
	{
		scale_ = Vector3::One;
		attackDeleyTime = p->laser.shotIntervalNormal / attackSpeedMul_;
		shotCount_ = p->laser.shotCountMult;
		break;
	}
	case Mode::enCharge:
	{
		scale_ = p->laser.chargeScale;
		attackDeleyTime = p->laser.shotIntervalCharge / attackSpeedMul_;
		shotCount_ = p->laser.shotCountNormal;
		break;
	}
	default:
		break;
	}
}

void LaserState::Enter()
{
	// 攻撃速度倍率を保持しておく（Setup()や発火済みタイマーのコールバック内でも同じ値を使うため）
	attackSpeedMul_ = GetAttackSpeedMul();

	// 攻撃モード
	{
		// 結果をランダムに
		srand(time(nullptr));
		// 0～9の乱数を取得
		uint8_t attackMode = rand() % enWeightMax;

		/* 抽選処理 */
		uint8_t currentWeightSum = INT_ZERO;
		for (uint8_t i = INT_ZERO; i < Mode::enMax; ++i) {
			currentWeightSum += weights_[i]; // 重みを足していく

			// 乱数が現在の重みの合計値未満なら当選
			if (attackMode < currentWeightSum) {
				mode_ = static_cast<Mode>(i);
				break; // 当選したらループを抜ける
			}
		}
	}

	// 初期化
	{
		isFinished_ = false;
		boss_->SetMoveVelocity(Vector3::Zero);
	}

	const auto* p = ParameterManager::Get().GetBossStateParam();

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(p->common.rotateSpeed);
	boss_->SetTargetRot(targetRot);

	// 発射タイミングや攻撃範囲を設定
	Setup();

	// タスクスケジューラを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	const float laserAttackTime = p->laser.attackTime / attackSpeedMul_; // [timing] 発射開始から判定・エフェクトを消すまでの秒数

	// 攻撃回数分だけ追加
	for (int i = INT_ZERO; i < shotCount_; ++i)
	{
		/***** 発射前 *****/
		taskScheduler_->AddTimer(shotTime_, [&]()
			{
				// タイマー発火時点で取り直す（Enter()のローカル変数pは寿命が切れているため使えない）
				const auto* p = ParameterManager::Get().GetBossStateParam();

				// タイマー発火時にプレイヤー座標を取得する
				targetPos_ = boss_->GetTargetPos();

				// 予測サークルインジケーターを生成
				if (attackRangeIndicator_) {
					DeleteGO(attackRangeIndicator_);
					attackRangeIndicator_ = nullptr;
				}
				float indicatorRadius = (mode_ == Mode::enCharge)
					? p->laser.indicatorRadiusCharge : p->laser.indicatorRadiusNormal;
				attackRangeIndicator_ = NewGO<AttackRange>(0, "laserRange");
				AttackRange::InitParam laserParam;
				laserParam.type      = AttackRange::Type::enCircle;
				laserParam.character = AttackRange::Character::Boss;
				// カウントダウン半分の時点で外円到達、残り半分は外円に張り付いて表示
				// ※攻撃速度を反映した実測の「予測→発射」秒数で割ることで、インジケーターの動きも攻撃の速さに追従する
				const float laserCountdown = (mode_ == Mode::enCharge)
					? (p->laser.shotIntervalCharge / attackSpeedMul_)
					: (p->laser.shotIntervalNormal / attackSpeedMul_);
				laserParam.pulseSpeed = 2.0f / laserCountdown;
				attackRangeIndicator_->SetInitParam(laserParam);
				attackRangeIndicator_->SetPosition(targetPos_);
				attackRangeIndicator_->SetScale(Vector3(indicatorRadius, 1.0f, indicatorRadius));
				attackRangeIndicator_->SetDraw(true);

				boss_->PlayAnimation(BossAnimID::enAnimAntic, GetAnimationSpeedMul());
			});

		shotTime_ += attackDeleyTime; // 予測 → 発射の間隔（Setup()で既に攻撃速度反映済み）

		/***** 発射中 *****/
		taskScheduler_->AddTimer(shotTime_, [&]()
			{
				// タイマー発火時点で取り直す（Enter()のローカル変数pは寿命が切れているため使えない）
				const auto* p = ParameterManager::Get().GetBossStateParam();

				// 計算
				float effectSize = p->laser.collisionScale * p->common.damageRingEffectScale; // エフェクトのサイズ
				Vector3 effectScale = scale_ * effectSize; // エフェクトのスケール
				float collisionScale = p->laser.collisionScale * scale_.x; // コリジョンのスケール
				uint32_t collisionId; // コリジョンのID

				// モードによって、サウンドの再生とコリジョンのID設定
				if (mode_ == Mode::enCharge) {
					collisionId = CharacterID::BossLaserStrongAtkID();
					SoundManager::Get().PlaySE(enSoundKind_Boss_Thunder_Strong);
				}
				else {
					collisionId = CharacterID::BossLaserWeakAtkID();
					SoundManager::Get().PlaySE(enSoundKind_Boss_Thunder_Weak);
				}
				// 予測サークルインジケーターを削除
				if (attackRangeIndicator_) {
					DeleteGO(attackRangeIndicator_);
					attackRangeIndicator_ = nullptr;
				}
				phase_ = Phase::enShot;
				boss_->PlayAnimation(BossAnimID::enAnimClicked, GetAnimationSpeedMul());

				// レーザーのエフェクトを生成
				laserEffectHandle_ = EffectManager::Get().PlayEffect(
					enEffectKind_Raser,
					targetPos_,  // 予測タイマー内で更新済みの座標を使う
					boss_->GetTransformRotation(),
					effectScale,
					GetEffectSpeedMul()
				);

				// 攻撃用コリジョンを生成
				attackHitbox_ = std::make_unique<GhostBody>();
				attackHitbox_->CreateSphere(
					boss_,
					collisionId,
					collisionScale,
					ghost::CollisionAttribute::BossAtk,
					ghost::CollisionAttributeMask::BossAtk
				);
				// コリジョンの座標設定
				attackHitbox_->SetPosition(targetPos_);

				// 草を曲げる
				if (GrassBendManager::IsInitialized())
				{
					const std::string bendKey = (mode_ == Mode::enCharge) ? "LaserStrong" : "LaserWeak";
					if (const auto* gp = ParameterManager::Get().GetGrassBendParam(bendKey))
					{
						GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
						GrassBendManager::Get().AddSource(targetPos_, params);
					}
				}
			});

		/* 発射後 */
		taskScheduler_->AddTimer(shotTime_ + laserAttackTime, [&]()
			{
				attackHitbox_.reset();
				EffectManager::Get().StopEffect(laserEffectHandle_);
			});

		shotTime_ += laserAttackTime; // 次の予測開始タイミングへ

	}

	/***** 終了 *****/
	taskScheduler_->AddTimer(shotTime_, [&]()
		{
			phase_ = Phase::enDone;
			isFinished_ = true;
		});
}

void LaserState::Update()
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void LaserState::Exit()
{
	scale_ = Vector3::Zero;
	shotTime_ = 0.0f;
	shotCount_ = 0;
	attackHitbox_.reset();
	taskScheduler_.reset();

	if (attackRangeIndicator_) {
		DeleteGO(attackRangeIndicator_);
		attackRangeIndicator_ = nullptr;
	}
}


/*==========================================*/
// 死亡状態
/*==========================================*/

void BossDeathState::Enter()
{
	// 死亡アニメーション
	boss_->PlayAnimation(BossAnimID::enAnimDeath);

	// 初期化
	isFinished_ = false; 

	// タスクスケジューラを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// アニメーションが再生されてから特定の時間たったら（Deathは「攻撃」ではないので攻撃速度の影響は受けない）
	taskScheduler_->AddTimer(ParameterManager::Get().GetBossStateParam()->death.animationTime, [&]()
		{
			BossStatus* status = boss_->GetStatus()->As<BossStatus>();
			status->Die();
			isFinished_ = true;
		});
}

void BossDeathState::Update()
{
	// 移動速度を0に
	boss_->SetMoveVelocity(Vector3::Zero);

	// 時間を計算
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void BossDeathState::Exit()
{
	taskScheduler_.reset();
}


/*==========================================*/
// 共通処理
/*==========================================*/

Vector3 BossStateBase::CalcMovePlayerVelocity(float speed)
{
	// 方向を取得
	Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
	diff.y = 0;

	// 方向ベクトルを取得
	if (diff.LengthSq() > 0.001f){ diff.Normalize(); } // 遠い場合は近づく
	else{ diff = Vector3::Zero; } // 近すぎる場合はピタッと止まる


	// スピードとフレームレート(DeltaTime)を掛けて返す
	return diff * speed * g_gameTime->GetFrameDeltaTime();
}

Vector3 BossStateBase::CalcVelocityTowards(Vector3 targetPos, float speed)
{
	// ゴールまでの方向ベクトルを計算（ゴール座標 - 今の座標）
	Vector3 diff = targetPos - boss_->GetTransformPosition();

	// 1フレームで進む距離（スピード × DeltaTime）
	float moveAmount = speed * g_gameTime->GetFrameDeltaTime();

	// ゴールまでの距離が、1フレームで進む距離より短ければ（到着寸前なら）
	if (diff.Length() <= moveAmount)
	{
		// 行き過ぎないように、残りの距離だけ進む速度を返す
		return diff;
	}

	// 遠い場合は正規化してスピードを掛ける
	diff.Normalize();
	return diff * moveAmount;
}


Quaternion BossStateBase::RotateToTarget(float rotateSpeed)
{
	// 1. 目的地(Target) － 現在地(Boss) で方向ベクトルを取得
	Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
	diff.y = 0.0f;
	Quaternion currentRot = boss_->GetTargetRot();

	if (diff.LengthSq() > 0.001f)
	{
		diff.Normalize();

		// 目標の角度を計算
		float angle = atan2(diff.x, diff.z);
		Quaternion targetRot;
		Quaternion resultRot;
		targetRot.SetRotationY(angle);

		// 現在の角度から目標の角度へ、指定スピードで滑らかに回転（Slerp）
		resultRot.Slerp(rotateSpeed, targetRot, currentRot);

		return resultRot;
	}

	return currentRot;
}

float BossStateBase::GetAttackSpeedMul() const
{
	auto* status = boss_->GetStatus()->As<BossStatus>();
	return status ? status->GetAttackSpeedMul() : 1.0f;
}

float BossStateBase::GetAnimationSpeedMul() const
{
	auto* status = boss_->GetStatus()->As<BossStatus>();
	return status ? status->GetAnimationSpeedMul() : 1.0f;
}

float BossStateBase::GetEffectSpeedMul() const
{
	auto* status = boss_->GetStatus()->As<BossStatus>();
	return status ? status->GetEffectSpeedMul() : 1.0f;
}