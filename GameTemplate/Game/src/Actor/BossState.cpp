#include "stdafx.h"
#include "BossState.h"
#include "BossCharacter.h"

namespace
{
	constexpr float BOSS_RUN_MOVE_SPEED = 100.0f;
	constexpr float BOSS_ROTATE_SPEED = 0.01f;

	// NPCControllerで定義している距離と同じ値を設定
	constexpr float SHORT_DISTANCE = 500.0f;
	constexpr float MID_DISTANCE = 1000.0f;
	constexpr float LONG_DISTANCE = 1500.0f;
}


/*==========================================*/
// 待機の状態
/*==========================================*/

void BossIdleState::Enter() 
{
	isFinished = false;
	boss_->PlayAnimation(BossAnimID::enAnimIdle);

	// 待機時間の生成
	{
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// 時間設定
		taskScheduler_->AddTimer(10.0f, [&]() {
			isFinished = true;
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
	isFinished = false; // 初期化
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
}

void BossRunState::Update()
{
	// ステートを抜けるかどうか
	{
		Vector3 diff = boss_->GetTargetPos() - boss_->GetTransformPosition();
		diff.y = 0.0f;

		if (diff.LengthSq() <= goalPos_)
		{
			isFinished = true;
			boss_->SetMoveVelocity(Vector3::Zero); // ピタッと止まる
			boss_->PlayAnimation(BossAnimID::enAnimIdle); // 待機アニメーション開始
			return;
		}
	}


	// 移動 / 回転の計算・設定
	{
		Vector3 moveVelocity = CalcMovementVelocity(BOSS_RUN_MOVE_SPEED);
		Quaternion moveRotate = RotateToTarget(BOSS_ROTATE_SPEED);
		boss_->SetMoveVelocity(moveVelocity);
		boss_->SetTargetRot(moveRotate);
	}
}

void BossRunState::Exit()
{
}

/*==========================================*/
// 通常攻撃状態
/*==========================================*/

void BossAttackState::Enter()
{
	isFinished = false; // 初期化
	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);// 攻撃の前にプレイヤーがいる方向へ向く
	boss_->SetTargetRot(targetRot);

	// タスクシステムを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 攻撃までの時間設定
	{
		// 攻撃の範囲を表すものを出す
		taskScheduler_->AddTimer(0.1f, [&]() {
			
			});

		// アニメーションの再生とコリジョンの生成
		taskScheduler_->AddTimer(2.0f, [&]() {
			boss_->PlayAnimation(BossAnimID::enAnimAttack); // 通常攻撃アニメーションを設定
			
			// ゴーストコリジョンを生成
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossAtkID(), 150.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);

			// 座標計算
			Vector3 bossPos = boss_->transform_.position;				// プレイヤーの現在の座標を取得

			const Matrix& mat = boss_->transform_.GetWorldMatrix(); // ボスのワールド座標を取得
			Vector3 forward(mat.m[2][0], mat.m[2][1], mat.m[2][2]); // Z軸
			forward.Normalize(); // ベクトルの長さを1にしておく

			Quaternion bossRot = boss_->GetTargetRot();
			float forwardOffset = 200.0f;									// 目の前にどれくらいズラすか
			float heightOffset = 50.0f;										// 高さの調整
			Vector3 targetPos = bossPos + (forward * forwardOffset);	// 前方向の座標を決定
			targetPos.y += heightOffset;									// 高さを決定

			// コリジョンの座標を設定
			attackHitbox_->SetPosition(targetPos);
			});

		// コリジョンを破棄
		taskScheduler_->AddTimer(2.5f, [&]()
			{
				attackHitbox_.reset(nullptr);
			},
			true);

		// 処理を終わる
		taskScheduler_->AddTimer(3.0f, [&]()
			{
				isFinished = true;
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

Vector3 BossStateBase::CalcMovementVelocity(float speed)
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
