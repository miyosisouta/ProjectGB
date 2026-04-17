#include "stdafx.h"
#include "Utility.h"
#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"

namespace 
{
	/* 回避 */
	constexpr float TARGET_POS_FORWARD = 300.0f;
	constexpr float AVOID_MOVE_SPEED = 1000.0f;
}


void Avoid::Enter(Character* p)
{
	// 回避のフラグを初期化
	isFinished_ = false;
	p->GetStateMachine()->ClearAvoidRequest();

	// 前フレームの移動量をリセット
	p->SetMoveVelocity(Vector3::Zero);


	// タスクスケージュールを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	taskScheduler_->AddTimer(0.1f, [&, p]()
		{
			
			Vector3 playerPos = p->transform_.position; // プレイヤーの現在の座標を取得
			Quaternion playerRot = p->GetStateMachine()->GetRotation(); // プレイヤーの回転を取得
			Vector3 forwardDir = p->GetStateMachine()->GetDirection();// プレイヤーが最後に向いていた方向を取得
			
			p->PlayAnimation(static_cast<int> (PlayerStateID::Avoid));
			playerRot.AddRotationDegY(180.0f);
			EffectManager::Get().PlayEffect(enEffectKind_Avoid, playerPos, playerRot, Vector3(3.0f, 3.0f, 3.0f));

			// 移動先の設定
			targetPos_ = playerPos + (forwardDir * TARGET_POS_FORWARD);

			// プレイヤーステータスの取得、無敵を設定
			PlayerStatus* status = p->GetStatus()->As<PlayerStatus>();
			status->AddInvincible(PlayerStatus::InvincibleFlags::enAvoid);

			// SEの再生
			SoundManager::Get().PlaySE(enSoundKind_Player_Utility);
		});

	taskScheduler_->AddTimer(0.8f, [&, p]()
		{
			// プレイヤーステータスの取得、無敵を削除
			PlayerStatus* status = p->GetStatus()->As<PlayerStatus>();
			status->RemoveInvincible(PlayerStatus::InvincibleFlags::enAvoid);

			// ステートを強制終了
			isFinished_ = true;
		});
}

void Avoid::Update(Character* p)
{
	// スケジューラを設定
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

	// 移動処理
	{
		// ゴールまでの方向ベクトルを計算（ゴール座標 - 今の座標）
		Vector3 diff = targetPos_ - p->GetTransformPosition();

		// 1フレームで進む距離（スピード × DeltaTime）
		float moveAmount = AVOID_MOVE_SPEED * g_gameTime->GetFrameDeltaTime();

		Vector3 moveVelocity;
		// ゴールまでの距離が、1フレームで進む距離より短ければ（到着寸前なら）
		if (diff.Length() <= moveAmount) {
			// 行き過ぎないように、残りの距離だけ進む速度を返す
			moveVelocity = diff;
		}
		else {
			// 遠い場合は正規化してスピードを掛ける
			diff.Normalize();
			moveVelocity = diff * moveAmount;
		}
		// 移動速度を設定
		p->SetMoveVelocity(moveVelocity);
	}
}

void Avoid::Exit(Character* p)
{
	// プレイヤーステータスの取得、無敵を削除
	PlayerStatus* status = p->GetStatus()->As<PlayerStatus>();
	status->RemoveInvincible(PlayerStatus::InvincibleFlags::enAvoid);

	p->SetMoveVelocity(Vector3::Zero);	// ピタッと止める

	taskScheduler_.reset(nullptr);
}
