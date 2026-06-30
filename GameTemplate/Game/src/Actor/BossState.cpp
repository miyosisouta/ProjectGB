#include "stdafx.h"
#include "BossState.h"
#include "BossCharacter.h"
#include "AttackRange.h"
#include "src/Actor/AttackObjectManager.h"
#include "src/Stage/GrassBendManager.h"
#include <time.h>

namespace
{
	// NPCControllerで定義している距離と同じ値を設定
	constexpr float SHORT_DISTANCE = 500.0f;	// 近距離
	constexpr float MID_DISTANCE = 1000.0f;		// 中距離
	constexpr float LONG_DISTANCE = 1500.0f;	// 遠距離

	// 待機
	constexpr float BOSS_IDLE_END_TIME = 6.0f; // 待機時間終了

	// ダッシュ
	static const Vector3 BOSS_RUN_EFFECT_SCALE = Vector3(20.0f, 20.0f, 20.0f); // エフェクトサイズ
	constexpr float BOSS_RUN_MOVE_SPEED = 100.0f;			// 走るときのベース速度
	constexpr float BOSS_RUN_SE_LOOP_SEQUENCE = 0.2f;		// 走る際のSEのループシーケンス
	constexpr float BOSS_RUN_EFFECT_LOOP_SEQUENCE = 0.5f;	// 走る際のエフェクトのループシーケンス
	
	// 通常攻撃
	constexpr float ATTACK_TASK_BEGIN_TIME = 0.1f;			// タスクスケジューラー最初の処理が始まるの時間
	constexpr float ATTACK_TASK_COLLISION_RESET_TIME = 0.6f;// タスクスケジューラーのコリジョンを消す時間
	constexpr float ATTACK_TASK_END_TIME = 1.0f;			// タスクスケジューラーの処理が終わる時間
	constexpr float ATTACK_COLLISION_FORWARD = 200.0f;		// 前方向
	constexpr float ATTACK_COLLISION_HEIGHT = 100.0f;		// 高さ
	constexpr float ATTACK_NORMAL_COLLISION_SIZE = 200.0f;	// コリジョンサイズ

	// ヒットスタンプ
	static Vector3 ATTACK_HEIGHT = Vector3(0.0f,3000.0f,0.0f);	// 高さ
	constexpr float BOSS_HITSTAMP_UP_BEGIN_TIME = 0.5f;			// ジャンプする時間
	constexpr float BOSS_HITSTAMP_OVERHEAD_MOVE_TIME = 2.0f;	// 頭上で移動する時間
	constexpr float BOSS_HITSTAMP_FALL_BEGIN_TIME = 5.0f;		// 落ち始める時間
	constexpr float BOSS_HITSTAMP_VERTICAL_VELOCITY = 1500.0f;	// 垂直速度
	constexpr float BOSS_HITSTAMP_UP_SPEED = 600.0f;			// ヒットスタンプ時飛び上がる際のベース速度
	constexpr float BOSS_HITSTAMP_DOWN_SPEED = 800.0f;			// ヒットスタンプ時着地する際のベース速度
	constexpr float ATTACK_HITSTAMP_COLLISION_SIZE = 350.0f;	// コリジョンサイズ
	constexpr float ATTACK_HITSTAMP_RANGE_SIZE = 70.0f;	// 攻撃範囲サイズ
	constexpr float GRAVITY_POWER = -800.0f;					// 重力の強さ
	constexpr float EFFECT_SCALE_HITSTAMP = 0.2f;				// ヒットスタンプのエフェクトサイズ
	
	// 回転攻撃
	constexpr float SPIN_ATTACK_START_TIME = 3.0f;				// 攻撃開始時間
	constexpr float SPIN_ATTACK_END_TIME = 7.5f;				// 攻撃終了時間
	constexpr float BOSS_SPIN_ATTACK_SPEED = 300.0f;			// 回転攻撃のベース速度
	constexpr float OVER_MOVE_DISTANCE_SPIN_ATTACK = 300.0f;	// 回転攻撃のターゲットの位置からさらに進む距離
	constexpr float ATTACK_SPIN_COLLISION_SIZE = 250.0f;		// 回転攻撃のコリジョンサイズ
	constexpr float ATTACK_SPIN_RANGE_SIZE = 50.0f;				// 攻撃予測ラインのサイズ
	constexpr float ATTACK_SPIN_INDICATOR_LENGTH = 250.0f;		// 攻撃予測ラインの長さ (見た目のみ、攻撃距離と無関係)
	constexpr float ATTACK_SPIN_INDICATOR_FORWARD = 500.0f;		// インジケーター中心の前方オフセット
	constexpr float BOSS_SPIN_SE_LOOP_SEQUENCE = 0.3f;			// 回転攻撃のSEのループシーケンス
	constexpr float BOSS_SPIN_EFFECT_LOOP_SEQUENCE = 1.0f;		// 回転攻撃のエフェクトのループシーケンス

	// 岩を投げる攻撃
	constexpr float ROCK_THROW_INDICATOR_LENGTH = 250.0f;		// 攻撃予測ラインの長さ (見た目のみ、攻撃距離と無関係)
	constexpr float ROCK_THROW_INDICATOR_BASE_SIZE = 200.0f;	// タイリング基準サイズ (長さ軸)
	constexpr float ROCK_THROW_INDICATOR_FORWARD = 600.0f;		// インジケーター中心の前方オフセット
	constexpr float ROCK_THROW_RANGE_SIZE = 33.0f;				// 攻撃予測ラインの横幅サイズ
	constexpr float ROCK_THROW_BEGIN_TIME = 2.5f;				// 岩を投げる時間
	constexpr float ROCK_THROW_END_TIME = 4.0f;					// 処理の終了時間
	constexpr float OVER_MOVE_DISTANCE_THROW_ROCK = 200.0f;		// プレイヤーのいる位置からさらに奥
	constexpr float ATTACK_ROCK_COLLISION_SIZE = 100.0f;		// コリジョンサイズ

	// レーザー
	constexpr float LASER_ATTACK_TIME = 1.0f;		// レーザーの攻撃時間
	constexpr float LASER_SHOT_TIME_NORMAL = 3.0f;	// レーザーの攻撃まで時間(通常)
	constexpr float LASER_SHOT_TIME_CHARGE = 6.0f;	// レーザーの攻撃まで時間(チャージ) 
	constexpr uint8_t LASER_SHOT_COUNT_NORMAL = 1;	// レーザーの本数(通常)
	constexpr uint8_t LASER_SHOT_COUNT_MULT = 3;	// レーザーの本数(マルチモード)
	static const Vector3 LASER_SHOT_SCALE_CHARGE = Vector3(1.75f, 1.75f, 1.75f); // レーザーのチャージ時の大きさ
	constexpr float LASER_SHOT_COLLISION_SCALE = 180.0f; // レーザーのhitCollisonのサイズ
	constexpr float LASER_INDICATOR_RADIUS_NORMAL = 45.0f; // 予測サークルの半径 (通常/連発)
	constexpr float LASER_INDICATOR_RADIUS_CHARGE = 75.0f; // 予測サークルの半径 (チャージ)
	constexpr float EFFECT_SCALE_FACTOR_LASER = 0.5f; // レーザーエフェクトサイズ調整

	// 死亡
	constexpr float DEATH_ANIMATION_TIME = 2.0f;

	// 共通
	constexpr float EFFECT_SCALE_FACTOR_DAMAGE_LING = 0.4f;		// 円形の攻撃予測用エフェクトのサイズ調整
	constexpr uint8_t INT_ZERO = 0;				// 初期化の値(int)
	constexpr float FLOAT_ZERO = 0.0f;			// 初期化の値(float)
	constexpr float BOSS_ROTATE_SPEED = 0.01f;	// ボスの回転速度
	constexpr float BOSS_ROTATE_MAX = 360.0f;	// 1回転
	constexpr float MOVE_EPSILON = 0.001f;		// 少しでも動いているか
	constexpr float DIST_HALF = 0.5f;			// 距離の半分
	constexpr float FIXED_EFFECT_SCALE_Y = 1.0f;// エフェクトのｙスケールの初期化
	constexpr float INIT_SHOT_TIME = 0.1f;		// タスクスケジューラの次の処理までの時間の初期化
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
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// 時間設定
		taskScheduler_->AddTimer(BOSS_IDLE_END_TIME, [&]() {
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
	isFinished_ = false; // 初期化
	boss_->PlayAnimation(BossAnimID::enAnimRun); // 走るアニメーションを設定

	// プレイヤーとの距離を求める
	Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
	diff.y = FLOAT_ZERO;
	float currentDistSq = diff.LengthSq(); // 今の距離の2乗

	// 射程範囲外から走り始めた → 遠距離の境界まで走る
	if (currentDistSq > LONG_DISTANCE * LONG_DISTANCE) {
		goalPos_ = LONG_DISTANCE * LONG_DISTANCE;
	}
	// 遠距離から走り始めた → 中距離の境界まで走る
	else if (currentDistSq > MID_DISTANCE * MID_DISTANCE) {
		goalPos_ = MID_DISTANCE * MID_DISTANCE;
	}
	// 中距離から走り始めた → 近距離の境界まで走る
	else if (currentDistSq > SHORT_DISTANCE * SHORT_DISTANCE) {
		goalPos_ = SHORT_DISTANCE * SHORT_DISTANCE;
	}

	// 音の再生
	{
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		const int SoundId = taskScheduler_->CreateLoopSequence(BOSS_RUN_SE_LOOP_SEQUENCE);
		taskScheduler_->AddLoopTimer(SoundId, FLOAT_ZERO, [&](void) {
			SoundManager::Get().PlaySE(enSoundKind_Boss_Run);
			});
	}

	// エフェクト
	{
		const int SoundId = taskScheduler_->CreateLoopSequence(BOSS_RUN_EFFECT_LOOP_SEQUENCE);
		taskScheduler_->AddLoopTimer(SoundId, FLOAT_ZERO, [&](void) {
			const Vector3 bossPos = boss_->transform_.position;
			const Quaternion bossRot = boss_->transform_.rotation;
			EffectManager::Get().PlayEffect(enEffectKind_Dash_Wind, bossPos, bossRot, BOSS_RUN_EFFECT_SCALE);
			});
	}
}

void BossRunState::Update()
{
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
		Vector3 moveVelocity = CalcMovePlayerVelocity(BOSS_RUN_MOVE_SPEED);
		Quaternion moveRotate = RotateToTarget(BOSS_ROTATE_SPEED);
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
	isFinished_ = false; // 初期化
	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);// 攻撃の前にプレイヤーがいる方向へ向く
	boss_->SetTargetRot(targetRot);

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 攻撃までの時間設定
	{
		// アニメーションの再生とコリジョンの生成
		taskScheduler_->AddTimer(ATTACK_TASK_BEGIN_TIME, [&]() {
			boss_->PlayAnimation(BossAnimID::enAnimAttack); // 通常攻撃アニメーションを設定
			SoundManager::Get().PlaySE(enSoundKind_Boss_NormalAttack); // 音の再生

			// ゴーストコリジョンを生成
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossNormalAtkID(), ATTACK_NORMAL_COLLISION_SIZE, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			
			// 座標計算
			Vector3 bossPos = boss_->transform_.position;				// プレイヤーの現在の座標を取得

			const Matrix& mat = boss_->transform_.GetWorldMatrix(); // ボスのワールド座標を取得
			Vector3 forward(mat.m[2][0], mat.m[2][1], mat.m[2][2]); // Z軸
			forward.Normalize(); // ベクトルの長さを1にしておく

			Quaternion bossRot = boss_->GetTargetRot();
			float forwardOffset = ATTACK_COLLISION_FORWARD;									// 目の前にどれくらいズラすか
			float heightOffset = ATTACK_COLLISION_HEIGHT;										// 高さの調整
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
			Vector3 collisionTargetScal = ATTACK_NORMAL_COLLISION_SIZE * EFFECT_SCALE_FACTOR_DAMAGE_LING;
			bossRot.AddRotationDegY(BOSS_ROTATE_MAX);
			EffectManager::Get().PlayEffect(enEffectKind_Wind_Blast_Boss, collisionTargetPos, bossRot, collisionTargetScal);
			});

		// コリジョンを破棄
		taskScheduler_->AddTimer(ATTACK_TASK_COLLISION_RESET_TIME, [&]()
			{
				attackHitbox_.reset(nullptr);
			},
			true);

		// 処理を終わる
		taskScheduler_->AddTimer(ATTACK_TASK_END_TIME, [&]()
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
	isFinished_ = false; // 初期化
	createAttackCollision_ = false; // コリジョンの生成を可能にする
	gravity_ = GRAVITY_POWER;
	phase_ = Phase::Ready; // 準備をする

	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);// 攻撃の前にプレイヤーがいる方向へ向く
	boss_->SetTargetRot(targetRot);

	boss_->PlayAnimation(BossAnimID::enAnimJump); // ジャンプアニメーションを設定

	// 攻撃範囲インジケーターを生成 (表示は Hover フェーズまで待つ)
	{
		attackRange_ = NewGO<AttackRange>(0, "hitStampRange");
		AttackRange::InitParam param;
		param.type      = AttackRange::Type::enCircle;
		param.character = AttackRange::Character::Boss;
		attackRange_->SetInitParam(param);
		attackRange_->SetScale(Vector3(ATTACK_HITSTAMP_RANGE_SIZE, 1.0f, ATTACK_HITSTAMP_RANGE_SIZE));
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 現在の位置から上方向に飛び上がる
	taskScheduler_->AddTimer(BOSS_HITSTAMP_UP_BEGIN_TIME, [&]() {
		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = boss_->GetTransformPosition() + ATTACK_HEIGHT; // 移動する地点を設定
		nextTargetPos_ = boss_->GetTargetPos() + ATTACK_HEIGHT;// 次に移動する地点を計算
		verticalVelocity_ = BOSS_HITSTAMP_VERTICAL_VELOCITY; // 垂直速度
		phase_ = Phase::JumpUp;
		});

	// プレイヤーの頭上へ移動
	taskScheduler_->AddTimer(BOSS_HITSTAMP_OVERHEAD_MOVE_TIME, [&]() {
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
			const float countdown = BOSS_HITSTAMP_FALL_BEGIN_TIME - BOSS_HITSTAMP_OVERHEAD_MOVE_TIME;
			attackRange_->SetPulseCountdown(countdown * 0.5f);
			attackRange_->SetDraw(true);
		}
		phase_ = Phase::Hover;
		});

	// 地面に落ちる
	taskScheduler_->AddTimer(BOSS_HITSTAMP_FALL_BEGIN_TIME, [&]() {

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
		Vector3 move = CalcVelocityTowards(targetPos_, BOSS_HITSTAMP_UP_SPEED);
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
		Vector3 move = CalcVelocityTowards(targetPos_, BOSS_HITSTAMP_DOWN_SPEED);
		move.y = FLOAT_ZERO;
		boss_->SetMoveVelocity(move);

		verticalVelocity_ += gravity_ * g_gameTime->GetFrameDeltaTime();
		boss_->transform_.localPosition.y += verticalVelocity_ * g_gameTime->GetFrameDeltaTime();

		if (boss_->GetTransformPosition().y <= FLOAT_ZERO)
		{
			boss_->transform_.localPosition.y = FLOAT_ZERO;
			verticalVelocity_ = FLOAT_ZERO;

			// ここに着地時の処理をまとめる
			boss_->SetMoveVelocity(Vector3::Zero);
			Vector3 targetPos = boss_->GetTransformPosition();
			Vector3 targetScal = ATTACK_HITSTAMP_COLLISION_SIZE * EFFECT_SCALE_HITSTAMP;
			EffectManager::Get().PlayEffect(enEffectKind_HitStamp, targetPos, boss_->GetTransformRotation(), targetScal);
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
	isFinished_ = false; // 初期化
	isAttackStart_ = false; // 攻撃範囲表示の間は攻撃を開始しない

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);
	boss_->SetTargetRot(targetRot);

	// 回転アニメーションを設定
	boss_->PlayAnimation(BossAnimID::enAnimSpin);

	// エフェクトを再生
	Vector3 targetScal = ATTACK_SPIN_COLLISION_SIZE * EFFECT_SCALE_FACTOR_DAMAGE_LING;
	spinEffectHandle_ = EffectManager::Get().PlayEffect(
		enEffectkind_Spin,
		boss_->GetTransformPosition(),
		boss_->GetTransformRotation(),
		targetScal
	);

	// プレイヤーの方向と進む距離を設定
	{
		Vector3 playerPos = boss_->GetTargetPos();			// プレイヤーの座標
		Vector3 bossPos = boss_->GetTransformPosition();	// ボスの座標

		Vector3 diff = playerPos - bossPos; // 方向を算出
		if (diff.LengthSq() >= MOVE_EPSILON)
		{
			diff.Normalize(); // 方向を正規化
			targetPos_ = playerPos + (diff * OVER_MOVE_DISTANCE_SPIN_ATTACK); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
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

			// ライン中心 = ボスから ATTACK_SPIN_INDICATOR_LENGTH の半分先
			Vector3 startPos = bossPos + (dir * (ATTACK_SPIN_INDICATOR_LENGTH * DIST_HALF + ATTACK_SPIN_INDICATOR_FORWARD));
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
			attackRangeIndicator_->SetScale(Vector3(ATTACK_SPIN_INDICATOR_LENGTH * 0.5f, FIXED_EFFECT_SCALE_Y, ATTACK_SPIN_RANGE_SIZE));
			attackRangeIndicator_->SetLineDrawProgress(0.0f); // 初期は非表示
			attackRangeIndicator_->SetDraw(true);
		}
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// タスクシステムの構築
	{
		// 移動開始（予測インジケーターを非表示にしてから攻撃）
		taskScheduler_->AddTimer(SPIN_ATTACK_START_TIME, [&]() {
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
			const int id = taskScheduler_->CreateLoopSequence(BOSS_SPIN_SE_LOOP_SEQUENCE);
			taskScheduler_->AddLoopTimer(id, FLOAT_ZERO, [&]() {
				SoundManager::Get().PlaySE(enSoundKind_Boss_Spin);
				});
			});


		// 5秒たったら強制終了
		taskScheduler_->AddTimer(SPIN_ATTACK_END_TIME, [&]() {
			boss_->SetMoveVelocity(Vector3::Zero);
			attackHitbox_.reset();
			isFinished_ = true;
			});
	}

}

void SpinState::Update()
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	// 攻撃開始前はラインインジケーターをボスから徐々に伸ばす
	if (!isAttackStart_ && attackRangeIndicator_)
	{
		predictionElapsed_ += g_gameTime->GetFrameDeltaTime();
		float progress = min(predictionElapsed_ / SPIN_ATTACK_START_TIME, 1.0f);
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
		Vector3 moveVelocity = CalcVelocityTowards(targetPos_, BOSS_SPIN_ATTACK_SPEED);
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
	isFinished_ = false; // 初期化
	boss_->SetMoveVelocity(Vector3::Zero);

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);
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
		targetPos_ = playerPos + (diff * OVER_MOVE_DISTANCE_THROW_ROCK); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
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

		float meshHalfExtent = ROCK_THROW_INDICATOR_LENGTH * DIST_HALF;
		Vector3 startPos = bossPos + targetDir * (ROCK_THROW_INDICATOR_LENGTH * DIST_HALF + ROCK_THROW_INDICATOR_FORWARD);
		startPos.y = 0.5f;

		float angle = atan2(targetDir.x, targetDir.z);
		Quaternion lineRot;
		lineRot.SetRotationY(angle);

		attackRangeIndicator_ = NewGO<AttackRange>(0, "rockRange");
		AttackRange::InitParam param;
		param.type      = AttackRange::Type::enLine;
		param.character = AttackRange::Character::Boss;
		param.baseSizeX = ROCK_THROW_INDICATOR_BASE_SIZE;
		attackRangeIndicator_->SetInitParam(param);
		attackRangeIndicator_->SetPosition(startPos);
		attackRangeIndicator_->SetRotation(lineRot);
		attackRangeIndicator_->SetScale(Vector3(meshHalfExtent, FIXED_EFFECT_SCALE_Y, ROCK_THROW_RANGE_SIZE));
		attackRangeIndicator_->SetLineDrawProgress(0.0f);
		attackRangeIndicator_->SetDraw(true);
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	taskScheduler_->AddTimer(ROCK_THROW_BEGIN_TIME, [this,bossPos,targetDir]()
		{
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

			float attackForward = ATTACK_COLLISION_FORWARD;
			float attackHeight = ATTACK_COLLISION_HEIGHT;
			Vector3 startPos = bossPos + (forward * attackForward);
			startPos.y = attackHeight;

			// 岩の生成
			AttackObjectManager::Get().CreateRock(
				boss_,
				startPos,
				targetDir,
				ATTACK_ROCK_COLLISION_SIZE
			);
			// アニメーションを再生
			boss_->PlayAnimation(BossAnimID::enAnimClicked);
		});

	taskScheduler_->AddTimer(ROCK_THROW_END_TIME, [&]()
		{
			isFinished_ = true;
		});
}

void ThrowRockState::Update()
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	if (attackRangeIndicator_)
	{
		predictionElapsed_ += g_gameTime->GetFrameDeltaTime();
		float progress = min(predictionElapsed_ / ROCK_THROW_BEGIN_TIME, 1.0f);
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
	shotTime_ = INIT_SHOT_TIME;
	attackDeleyTime = INIT_SHOT_TIME;

	switch (mode_) 
	{
	case Mode::enNormal:
	{
		scale_ = Vector3::One;
		attackDeleyTime = LASER_SHOT_TIME_NORMAL;
		shotCount_ = LASER_SHOT_COUNT_NORMAL;
		break;
	}
	case Mode::enMult:
	{
		scale_ = Vector3::One;
		attackDeleyTime = LASER_SHOT_TIME_NORMAL;
		shotCount_ = LASER_SHOT_COUNT_MULT;
		break;
	}
	case Mode::enCharge:
	{
		scale_ = LASER_SHOT_SCALE_CHARGE;
		attackDeleyTime = LASER_SHOT_TIME_CHARGE;
		shotCount_ = LASER_SHOT_COUNT_NORMAL;
		break;
	}
	default:
		break;
	}
}

void LaserState::Enter()
{
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
	

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);
	boss_->SetTargetRot(targetRot);

	// 発射タイミングや攻撃範囲を設定
	Setup();

	// タスクスケジューラを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();


	// 攻撃回数分だけ追加
	for (int i = INT_ZERO; i < shotCount_; ++i)
	{
		/***** 発射前 *****/
		taskScheduler_->AddTimer(shotTime_, [&]()
			{
				if (mode_ == Mode::enCharge) {
					int i = 0;
				}
				// タイマー発火時にプレイヤー座標を取得する
				targetPos_ = boss_->GetTargetPos();

				// 予測サークルインジケーターを生成
				if (attackRangeIndicator_) {
					DeleteGO(attackRangeIndicator_);
					attackRangeIndicator_ = nullptr;
				}
				float indicatorRadius = (mode_ == Mode::enCharge)
					? LASER_INDICATOR_RADIUS_CHARGE : LASER_INDICATOR_RADIUS_NORMAL;
				attackRangeIndicator_ = NewGO<AttackRange>(0, "laserRange");
				AttackRange::InitParam laserParam;
				laserParam.type      = AttackRange::Type::enCircle;
				laserParam.character = AttackRange::Character::Boss;
				// カウントダウン半分の時点で外円到達、残り半分は外円に張り付いて表示
				const float laserCountdown = (mode_ == Mode::enCharge) ? LASER_SHOT_TIME_CHARGE : LASER_SHOT_TIME_NORMAL;
				laserParam.pulseSpeed = 2.0f / laserCountdown;
				attackRangeIndicator_->SetInitParam(laserParam);
				attackRangeIndicator_->SetPosition(targetPos_);
				attackRangeIndicator_->SetScale(Vector3(indicatorRadius, 1.0f, indicatorRadius));
				attackRangeIndicator_->SetDraw(true);

				boss_->PlayAnimation(BossAnimID::enAnimAntic);
			});

		shotTime_ += attackDeleyTime; // 予測 → 発射の間隔

		/***** 発射中 *****/
		taskScheduler_->AddTimer(shotTime_, [&]()
			{
				// 計算
				float effectSize = LASER_SHOT_COLLISION_SCALE * EFFECT_SCALE_FACTOR_DAMAGE_LING; // エフェクトのサイズ
				Vector3 effectScale = scale_ * effectSize; // エフェクトのスケール
				float collisionScale = LASER_SHOT_COLLISION_SCALE * scale_.x; // コリジョンのスケール
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
				boss_->PlayAnimation(BossAnimID::enAnimClicked);

				// レーザーのエフェクトを生成
				laserEffectHandle_ = EffectManager::Get().PlayEffect(
					enEffectKind_Raser,
					targetPos_,  // 予測タイマー内で更新済みの座標を使う
					boss_->GetTransformRotation(),
					effectScale
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
		taskScheduler_->AddTimer(shotTime_ + LASER_ATTACK_TIME, [&]()
			{
				attackHitbox_.reset();
				EffectManager::Get().StopEffect(laserEffectHandle_);
			});

		shotTime_ += LASER_ATTACK_TIME; // 次の予測開始タイミングへ

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

	// アニメーションが再生されてから特定の時間たったら
	taskScheduler_->AddTimer(DEATH_ANIMATION_TIME, [&]()
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