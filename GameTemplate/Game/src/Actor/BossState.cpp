#include "stdafx.h"
#include "BossState.h"
#include "BossCharacter.h"

namespace
{
	// 動きのスピード
	constexpr float BOSS_RUN_MOVE_SPEED = 100.0f;		// 走るときのベース速度
	constexpr float BOSS_HITSTAMP_UP_SPEED = 600.0f;	// ヒットスタンプ時飛び上がる際のベース速度
	constexpr float BOSS_HITSTAMP_DOWN_SPEED = 800.0f;	// ヒットスタンプ時着地する際のベース速度
	constexpr float BOSS_SPIN_ATTACK_SPEED = 300.0f;	// 回転攻撃のベース速度
	constexpr float BOSS_ROTATE_SPEED = 0.01f;			// ボスの回転速度

	// NPCControllerで定義している距離と同じ値を設定
	constexpr float SHORT_DISTANCE = 500.0f;	// 近距離
	constexpr float MID_DISTANCE = 1000.0f;		// 中距離
	constexpr float LONG_DISTANCE = 1500.0f;	// 遠距離

	// 通常攻撃
	constexpr float ATTACK_COLLISION_FORWARD = 200.0f; // 前方向
	constexpr float ATTACK_COLLISION_HEIGHT = 50.0f;   // 高さ

	// ヒットスタンプ
	static Vector3 ATTACK_HEIGHT = Vector3(0.0f,800.0f,0.0f); // 高さ

	// 回転攻撃
	constexpr float OVER_MOVE_DISTANCE = 300.0f;
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
		Vector3 moveVelocity = CalcMovePlayerVelocity(BOSS_RUN_MOVE_SPEED);
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
			attackHitbox_->CreateSphere(boss_, CharacterID::BossNormalAtkID(), 150.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			
			// 座標計算
			Vector3 bossPos = boss_->transform_.position;				// プレイヤーの現在の座標を取得

			const Matrix& mat = boss_->transform_.GetWorldMatrix(); // ボスのワールド座標を取得
			Vector3 forward(mat.m[2][0], mat.m[2][1], mat.m[2][2]); // Z軸
			forward.Normalize(); // ベクトルの長さを1にしておく

			Quaternion bossRot = boss_->GetTargetRot();
			float forwardOffset = ATTACK_COLLISION_FORWARD;									// 目の前にどれくらいズラすか
			float heightOffset = ATTACK_COLLISION_HEIGHT;										// 高さの調整
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
// ヒットスタンプ
/*==========================================*/

void HitStampState::Enter()
{
	isFinished = false; // 初期化
	createAttackCollision_ = false; // コリジョンの生成を可能にする
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
		phase_ = Phase::JumpUp;
		});

	// プレイヤーの頭上へ移動
	taskScheduler_->AddTimer(2.0f, [&]() {
		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = nextTargetPos_;// 移動する地点を設定
		nextTargetPos_ = boss_->GetTargetPos();// 次に移動する地点を計算
		phase_ = Phase::Hover;
		});

	// 地面に落ちる
	taskScheduler_->AddTimer(4.0f, [&]() {
		boss_->SetMoveVelocity(Vector3::Zero);
		targetPos_ = nextTargetPos_;// 移動する地点を計算
		phase_ = Phase::FallDown;
		});

	// 地面に着地時
	taskScheduler_->AddTimer(6.0f, [&]() {
		boss_->SetMoveVelocity(Vector3::Zero);
		phase_ = Phase::ShokingStamp;
		});

	// 地面に着地後
	taskScheduler_->AddTimer(6.1f, [&]() {
		boss_->SetMoveVelocity(Vector3::Zero);
		phase_ = Phase::Finished;
		}, true);

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
		// 少しずつ上へ
		Vector3 moveUp = CalcVelocityTowards(targetPos_, BOSS_HITSTAMP_UP_SPEED);
		boss_->SetMoveVelocity(moveUp);
		break;
	}

	case Phase::Hover:
	{
		// プレイヤーの頭上へ移動
		Vector3 playerPos = boss_->GetTargetPos(); // プレイヤーの座標を取得
		Vector3 currentBossPos = boss_->GetTransformPosition(); // ボスの座標を取得

		targetPos_ = Vector3(playerPos.x, currentBossPos.y, playerPos.z); // プレイヤーの頭上へ座標を設定
		boss_->transform_.localPosition = targetPos_; // 指定した座標をトランスフォームに設定
		boss_->transform_.UpdateTransform(); // トランスフォームを更新
		break;
	}

	case Phase::FallDown:
	{
		// プレイヤーのいた場所へ落下
		Vector3 moveDown = CalcVelocityTowards(targetPos_, BOSS_HITSTAMP_DOWN_SPEED);
		boss_->SetMoveVelocity(moveDown);

		break;
	}

	case Phase::ShokingStamp:
	{
		// 作られてないならゴーストコリジョンを生成
		if (!createAttackCollision_) {
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossHitStampAtkID(), 250.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			attackHitbox_->SetPosition(targetPos_);
			boss_->SetMoveVelocity(Vector3::Zero);
			createAttackCollision_ = true;
		}

		break;
	}

	case Phase::Finished:
	{
		attackHitbox_.reset();
		isFinished = true;
		break;
	}
	}
}

void HitStampState::Exit()
{
	taskScheduler_.reset(nullptr);
}


/*==========================================*/
// 回転攻撃
/*==========================================*/

void SpinState::Enter()
{
	isFinished = false; // 初期化
	isAttackStart_ = false; // 攻撃範囲表示の間は攻撃を開始しない

	// 攻撃の前にプレイヤーがいる方向へ向く
	Quaternion targetRot = RotateToTarget(BOSS_ROTATE_SPEED);
	boss_->SetTargetRot(targetRot);

	// 回転アニメーションを設定
	boss_->PlayAnimation(BossAnimID::enAnimSpin); 

	// プレイヤーの方向と進む距離を設定
	{
		Vector3 playerPos = boss_->GetTargetPos();			// プレイヤーの座標
		Vector3 bossPos = boss_->GetTransformPosition();	// ボスの座標

		Vector3 diff = playerPos - bossPos; // 方向を算出
		if (diff.LengthSq() >= 0.001f)
		{
			diff.Normalize(); // 方向を正規化
			targetPos_ = playerPos + (diff * OVER_MOVE_DISTANCE); // プレイヤーのいる位置とボスから見た方向から少し先まで進む
		}
		else {
			targetPos_ = playerPos;
		}
	}

	// タスクシステムの構築
	{
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// 攻撃範囲を描画
		taskScheduler_->AddTimer(0.5f, [&]() {
			
			});

		// 移動開始
		taskScheduler_->AddTimer(1.5f, [&]() {
			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateSphere(boss_, CharacterID::BossSpinAtkID(), 110.0f, ghost::CollisionAttribute::BossAtk, ghost::CollisionAttributeMask::BossAtk);
			isAttackStart_ = true;
			});

		// 5秒たったら強制終了
		taskScheduler_->AddTimer(5.0f, [&]() {
			boss_->SetMoveVelocity(Vector3::Zero);
			attackHitbox_.reset();
			isFinished = true;
			});
	}
	
}

void SpinState::Update()
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	if (isAttackStart_) 
	{
		// 移動先を計算・設定
		Vector3 moveVelocity = CalcVelocityTowards(targetPos_, BOSS_SPIN_ATTACK_SPEED);
		boss_->SetMoveVelocity(moveVelocity);

		// ボスの現在の座標を取得、攻撃コリジョンの座標を更新
		Vector3 bossPos = boss_->GetTransformPosition();
		if (attackHitbox_) { attackHitbox_->SetPosition(bossPos); }

		if (moveVelocity.LengthSq() < 0.001f)
		{
			boss_->SetMoveVelocity(Vector3::Zero); // ピタッと止める
			attackHitbox_.reset();
			isFinished = true;                     // ステート終了！
		}
	}
}

void SpinState::Exit()
{
	taskScheduler_.reset(nullptr);
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
