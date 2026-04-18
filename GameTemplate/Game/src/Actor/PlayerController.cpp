#include "stdafx.h"
#include "PlayerController.h"
#include "src/Actor/Player.h"
#include "src/Actor/StateMachine.h"


namespace
{
	constexpr float DASH_HOLD_THRESHOLD = 0.15f; // 回避とダッシュの切り替え閾値
}


void PlayerController::Update()
{
	// プレイヤーがないなら処理を返す
	if (!target_) { return; }

	// ステートマシーンを取得
	auto* targetStateMachine = target_->GetStateMachine();

	// Aボタン、長押しでダッシュ、短押しで回避
	UpdateAButton(targetStateMachine);

	// Bボタンで通常攻撃
	targetStateMachine->ActionButtonB(g_pad[0]->IsPress(enButtonB));
	// Yボタンで特殊能力
	targetStateMachine->ActionButtonY(g_pad[0]->IsPress(enButtonY));
	// Xボタンで汎用能力
	targetStateMachine->ActionButtonX(g_pad[0]->IsPress(enButtonX));

	// スティックの入力を取得
	float stickX = g_pad[0]->GetLStickXF();
	float stickY = g_pad[0]->GetLStickYF();
	

	// 回転
	if (fabs(stickX) >= 0.01f || fabs(stickY) >= 0.01f)
	{
		// 左スティックの方向
		targetStateMachine->SetRotation(ComputeRotation());
		// 左スティックの入力量を取得
		targetStateMachine->SetDirection(GetStickL());
	}
	// スティックの入力量を設定する
	// 0～1の範囲
	targetStateMachine->SetStickLAmount(GetStickL().Length());
}


void PlayerController::UpdateAButton(StateMachine* sm)
{
	const bool isPressed = g_pad[0]->IsPress(enButtonA);
	const float deltaTime = g_gameTime->GetFrameDeltaTime();

	if (isPressed)
	{
		if (!aButtonHeld_)
		{
			avoidInputConsumed_ = false; // ✅ 押し始めにリセット
		}

		aButtonHeld_ = true;
		aButtonHeldTime_ += deltaTime;

		if (aButtonHeldTime_ >= DASH_HOLD_THRESHOLD)
		{
			sm->ActionButtonA(true);
		}
	}
	else
	{
		if (aButtonHeld_)
		{
			// 短押し かつ まだ消費していない場合だけリクエスト
			if (aButtonHeldTime_ < DASH_HOLD_THRESHOLD && !avoidInputConsumed_)
			{
				// ★ 枯渇中は回避フラグを立てない
				if (!target_->IsExhausted()) {
					sm->SetAvoidRequested(true);
				}
				sm->SetAvoidRequested(true);
				avoidInputConsumed_ = true;
			}

			sm->ActionButtonA(false);
			aButtonHeld_ = false;
			aButtonHeldTime_ = 0.0f;
		}
	}
}


Vector3 PlayerController::GetStickL()
{
	// 左スティックの入力量を取得
	Vector3 stickL;
	stickL.x = g_pad[0]->GetLStickXF();
	stickL.y = g_pad[0]->GetLStickYF();

	// カメラの前方向と右方向のベクトルを取得
	Vector3 forward = g_camera3D->GetForward();
	Vector3 right = g_camera3D->GetRight();

	// y方向には移動しない
	forward.y = 0.0f;
	right.y = 0.0f;

	// 左スティックの入力量を加算
	right *= stickL.x;
	forward *= stickL.y;

	Vector3 direction = right + forward;
	// 0～1の範囲に変更
	direction.Normalize();

	return direction;
}


Quaternion PlayerController::ComputeRotation()
{
	// スティックの方向
	Vector3 direction = GetStickL();
	// スティック入力を使ってY軸回転の情報を得る
	Quaternion q;
	q.SetRotationYFromDirectionXZ(direction);

	return q;
}
