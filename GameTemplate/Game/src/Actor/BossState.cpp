#include "stdafx.h"
#include "BossState.h"
#include "BossCharacter.h"
#include "src/Actor/AttackObjectManager.h"

namespace
{
	// NPCControllerで定義している距離と同じ値を設定
	constexpr float SHORT_DISTANCE = 500.0f;	// 近距離
	constexpr float MID_DISTANCE = 1000.0f;		// 中距離
	constexpr float LONG_DISTANCE = 1500.0f;	// 遠距離


	// 動きのスピード
	constexpr float BOSS_RUN_MOVE_SPEED = 100.0f;		// 走るときのベース速度
	constexpr float BOSS_HITSTAMP_UP_SPEED = 600.0f;	// ヒットスタンプ時飛び上がる際のベース速度
	constexpr float BOSS_HITSTAMP_DOWN_SPEED = 800.0f;	// ヒットスタンプ時着地する際のベース速度
	constexpr float BOSS_SPIN_ATTACK_SPEED = 300.0f;	// 回転攻撃のベース速度
	constexpr float BOSS_ROTATE_SPEED = 0.01f;			// ボスの回転速度

	
	// 通常攻撃
	constexpr float ATTACK_COLLISION_FORWARD = 200.0f; // 前方向
	constexpr float ATTACK_COLLISION_HEIGHT = 100.0f;   // 高さ

	// ヒットスタンプ
	static Vector3 ATTACK_HEIGHT = Vector3(0.0f,3000.0f,0.0f); // 高さ
	constexpr float GRAVITY_POWER = -800.0f; // 重力の強さ
	
	// 回転攻撃
	constexpr float OVER_MOVE_DISTANCE_SPIN_ATTACK = 300.0f;

	// 岩を投げる攻撃
	constexpr float OVER_MOVE_DISTANCE_THROW_ROCK = 200.0f;

	// コリジョンサイズ(Sphere)
	constexpr float ATTACK_NORMAL_COLLISION_SIZE = 200.0f;
	constexpr float ATTACK_HITSTAMP_COLLISION_SIZE = 350.0f;
	constexpr float ATTACK_SPIN_COLLISION_SIZE = 250.0f;
	constexpr float ATTACK_ROCK_COLLISION_SIZE = 100.0f;

	// エフェクトスケール変換
	constexpr float EFFECT_SCALE_FACTOR = 0.4f;

	// タスクスケジュールのループ
	constexpr float BOSS_RUN_SE_LOOP_SEQUENCE = 0.2f;	// 走る際のSEのループシーケンス
	constexpr float BOSS_RUN_EFFECT_LOOP_SEQUENCE = 0.5f; // 走る際のエフェクトのループシーケンス
	constexpr float BOSS_SPIN_SE_LOOP_SEQUENCE = 0.3f;	// 回転攻撃のSEのループシーケンス
	constexpr float BOSS_SPIN_EFFECT_LOOP_SEQUENCE = 1.0f;	// 回転攻撃のエフェクトのループシーケンス
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
		taskScheduler_->AddTimer(10.0f, [&]() {
			isFinished_ = true;
			});
	}
}

void BossIdleState::Update()
{
	boss_->SetMoveVelocity(Vector3::Zero); // 移動速度を0に

	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
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
	diff.y = 0.0f;
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
		taskScheduler_->AddLoopTimer(SoundId, 0.0f, [&](void) {
			SoundManager::Get().PlaySE(enSoundKind_Gorilla_Run);
			});
	}

	// エフェクト
	{
		const int SoundId = taskScheduler_->CreateLoopSequence(BOSS_RUN_EFFECT_LOOP_SEQUENCE);
		taskScheduler_->AddLoopTimer(SoundId, 0.0f, [&](void) {
			const Vector3 bossPos = boss_->transform_.position;
			const Quaternion bossRot = boss_->transform_.rotation;
			EffectManager::Get().PlayEffect(enEffectKind_Dash_Wind, bossPos, bossRot, Vector3(20.0f, 20.0f, 20.0f));
			});
	}
}

void BossRunState::Update()
{
	// ステートを抜けるかどうか
	{
		Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
		diff.y = 0.0f;

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
		taskScheduler_->AddTimer(0.1f, [&]() {
			boss_->PlayAnimation(BossAnimID::enAnimAttack); // 通常攻撃アニメーションを設定
			SoundManager::Get().PlaySE(enSoundKind_Gorilla_NormalAttack); // 音の再生

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

			// エフェクトのPRSを決め
			Vector3 collisionTargetScal = ATTACK_NORMAL_COLLISION_SIZE * EFFECT_SCALE_FACTOR;
			bossRot.AddRotationDegY(360.0f);
			EffectManager::Get().PlayEffect(enEffectKind_Wind_Blast_Boss, collisionTargetPos, bossRot, collisionTargetScal);
			});

		// コリジョンを破棄
		taskScheduler_->AddTimer(0.6f, [&]()
			{
				attackHitbox_.reset(nullptr);
			},
			true);

		// 処理を終わる
		taskScheduler_->AddTimer(1.0f, [&]()
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

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 現在の位置から上方向に飛び上がる
	taskScheduler_->AddTimer(0.5f, [&]() {
		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = boss_->GetTransformPosition() + ATTACK_HEIGHT; // 移動する地点を設定
		nextTargetPos_ = boss_->GetTargetPos() + ATTACK_HEIGHT;// 次に移動する地点を計算
		verticalVelocity_ = 1500.0f;
		phase_ = Phase::JumpUp;
		});

	// プレイヤーの頭上へ移動
	taskScheduler_->AddTimer(2.0f, [&]() {
		Vector3 collisionScale = ATTACK_HITSTAMP_COLLISION_SIZE * EFFECT_SCALE_FACTOR;
		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = nextTargetPos_;// 移動する地点を設定

		// このタイミングでプレイヤー座標を確定させ、以降は追従しない
		Vector3 playerPos = boss_->GetTargetPos();
		fixedAttackPos_ = Vector3(playerPos.x, 0.0f, playerPos.z); // 落下地点（地面）を固定

		// 攻撃予測エフェクトを固定地点に表示
		predictionEffectHandle_ = EffectManager::Get().PlayEffect(
			enEffectKind_DamageZone_Ring,
			fixedAttackPos_,
			boss_->GetTransformRotation(),
			collisionScale
		);
		phase_ = Phase::Hover;
		});

	// 地面に落ちる
	taskScheduler_->AddTimer(4.0f, [&]() {

		boss_->SetMoveVelocity(Vector3::Zero);
		verticalVelocity_ = 0.0f;

		// Hover時に確定した固定地点へ落下（プレイヤーが動いても追わない）
		targetPos_ = fixedAttackPos_;

		// 落下開始と同時に予測エフェクトを停止
		if (predictionEffectHandle_ != INVALID_EFFECT_HANDLE)
		{
			EffectManager::Get().StopEffect(predictionEffectHandle_);
			predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
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
		move.y = 0.0f;
		boss_->SetMoveVelocity(move);

		// Y軸は重力で管理
		verticalVelocity_ += gravity_ * g_gameTime->GetFrameDeltaTime();
		boss_->transform_.localPosition.y += verticalVelocity_ * g_gameTime->GetFrameDeltaTime();

		// 目標高度を超えたらHoverへ
		if (boss_->GetTransformPosition().y >= targetPos_.y)
		{
			verticalVelocity_ = 0.0f;
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
		move.y = 0.0f;
		boss_->SetMoveVelocity(move);

		verticalVelocity_ += gravity_ * g_gameTime->GetFrameDeltaTime();
		boss_->transform_.localPosition.y += verticalVelocity_ * g_gameTime->GetFrameDeltaTime();

		if (boss_->GetTransformPosition().y <= 0.0f)
		{
			boss_->transform_.localPosition.y = 0.0f;
			verticalVelocity_ = 0.0f;

			// ここに着地時の処理をまとめる
			boss_->SetMoveVelocity(Vector3::Zero);
			Vector3 targetPos = boss_->GetTransformPosition();
			Vector3 targetScal = ATTACK_HITSTAMP_COLLISION_SIZE * EFFECT_SCALE_FACTOR;
			EffectManager::Get().PlayEffect(enEffectKind_HitStamp, targetPos, boss_->GetTransformRotation(), targetScal);
			SoundManager::Get().PlaySE(enSoundKind_Gorilla_HitStamp);

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
	// 万が一エフェクトが残っていたら確実に停止する（FallDown以降で正常に止まるが保険として）
	if (predictionEffectHandle_ != INVALID_EFFECT_HANDLE)
	{
		EffectManager::Get().StopEffect(predictionEffectHandle_);
		predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
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
	Vector3 targetScal = ATTACK_SPIN_COLLISION_SIZE * EFFECT_SCALE_FACTOR;
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
		if (diff.LengthSq() >= 0.001f)
		{
			diff.Normalize(); // 方向を正規化
			targetPos_ = playerPos + (diff * OVER_MOVE_DISTANCE_SPIN_ATTACK); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
		}
		else {
			targetPos_ = playerPos;
		}
	}

	// ボスから targetPos_ まで1つのエフェクトを引き伸ばして攻撃予測ラインを表示
	{
		Vector3 bossPos = boss_->GetTransformPosition();
		Vector3 dir = targetPos_ - bossPos;
		float totalDist = dir.Length();

		if (totalDist >= 0.001f)
		{
			dir.Normalize();

			// エフェクトの中心
			Vector3 centerPos = bossPos + dir * (totalDist * 0.5f);
			centerPos.y = 0.0f;

			// 方向に合わせてY軸回転
			float angle = atan2(dir.x, dir.z);
			Quaternion predictionRot;
			predictionRot.SetRotationY(angle);

			// 大きさを計算
			float zScale = totalDist * EFFECT_SCALE_FACTOR;
			float xScale = ATTACK_SPIN_COLLISION_SIZE * EFFECT_SCALE_FACTOR;
			Vector3 predictionScale = Vector3(xScale, 1.0f, zScale);

			// エフェクトを再生
			predictionEffectHandle_ = EffectManager::Get().PlayEffect(
				enEffectKind_DamageZone_Ring,
				centerPos,
				predictionRot,
				predictionScale
			);
		}
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// タスクシステムの構築
	{
		// 移動開始（予測エフェクトを停止してから攻撃）
		taskScheduler_->AddTimer(1.5f, [&]() {
			// 予測エフェクトを停止
			if (predictionEffectHandle_ != INVALID_EFFECT_HANDLE)
			{
				EffectManager::Get().StopEffect(predictionEffectHandle_);
				predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
			}

			// コリジョンの生成
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossSpinAtkID(), 200.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			isAttackStart_ = true;

			// SEの再生
			const int id = taskScheduler_->CreateLoopSequence(BOSS_SPIN_SE_LOOP_SEQUENCE);
			taskScheduler_->AddLoopTimer(id, 0.0f, [&](void) {
				SoundManager::Get().PlaySE(enSoundKind_Gorilla_Spin);
				});
			});


		// 5秒たったら強制終了
		taskScheduler_->AddTimer(5.0f, [&]() {
			boss_->SetMoveVelocity(Vector3::Zero);
			attackHitbox_.reset();
			isFinished_ = true;
			});
	}

}

void SpinState::Update()
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

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

		if (moveVelocity.LengthSq() < 0.001f)
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

	// 予測エフェクトが残っていたら停止（タイマー起動前に強制終了した場合の保険）
	if (predictionEffectHandle_ != INVALID_EFFECT_HANDLE)
	{
		EffectManager::Get().StopEffect(predictionEffectHandle_);
		predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
	}

	taskScheduler_.reset(nullptr);
}


/*==========================================*/
// 岩を投げつける攻撃
/*==========================================*/

void ThrowRockState::Render(RenderContext& rc)
{
}

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
	if (diff.LengthSq() >= 0.001f)
	{
		diff.Normalize(); // 方向を正規化
		targetPos_ = playerPos + (diff * OVER_MOVE_DISTANCE_THROW_ROCK); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
	}
	else {
		targetPos_ = playerPos;
	}

	// 攻撃予測エフェクトの設定
	Vector3 targetDir = targetPos_ - bossPos; // ターゲットの方向
	float totalDist = targetDir.Length(); // 距離

	if (totalDist >= 0.001f)
	{
		targetDir.Normalize();

		// エフェクトの中心
		Vector3 centerPos = bossPos + targetDir * (totalDist * 0.5f);
		centerPos.y = 0.0f;

		// 方向に合わせてY軸回転
		float angle = atan2(targetDir.x, targetDir.z);
		Quaternion predictionRot;
		predictionRot.SetRotationY(angle);

		// 大きさを計算
		float zScale = totalDist * EFFECT_SCALE_FACTOR;
		float xScale = ATTACK_ROCK_COLLISION_SIZE * EFFECT_SCALE_FACTOR;
		Vector3 predictionScale = Vector3(xScale, 1.0f, zScale);

		// エフェクトを再生
		predictionEffectHandle_ = EffectManager::Get().PlayEffect(
			enEffectKind_DamageZone_Ring,
			centerPos,
			predictionRot,
			predictionScale
		);
	}

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	taskScheduler_->AddTimer(1.5f, [this,bossPos,targetDir]()
		{
			// エフェクトの再生を止める
			EffectManager::Get().StopEffect(predictionEffectHandle_);

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

	taskScheduler_->AddTimer(2.0f, [&]()
		{
			isFinished_ = true;
		});
}

void ThrowRockState::Update()
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void ThrowRockState::Exit()
{
	// 強制終了された場合の保険（タイマーが起動前に Exit が来るケース）
	if (predictionEffectHandle_ != INVALID_EFFECT_HANDLE)
	{
		EffectManager::Get().StopEffect(predictionEffectHandle_);
		predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
	}

	taskScheduler_.reset();
}

/*==========================================*/
// 死亡状態
/*==========================================*/

void BossDeathState::Enter()
{
}

void BossDeathState::Update()
{
	
}

void BossDeathState::Exit()
{
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