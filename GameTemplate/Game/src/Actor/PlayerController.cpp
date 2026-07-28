#include "stdafx.h"
#include "PlayerController.h"
#include "src/Actor/Player.h"
#include "src/Actor/StateMachine.h"
#include "src/Battle/TutorialManager.h"


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

	// 各スキルボタン（チュートリアルで解禁されるまでは入力そのものを無視する）
	const bool allowNormalAttack = TutorialManager::Get().IsActionAllowed(TutorialManager::Action::NormalAttack);
	const bool allowSkill        = TutorialManager::Get().IsActionAllowed(TutorialManager::Action::Skill);
	targetStateMachine->ActionButtonB(allowNormalAttack && KeyConfig::Get().IsPress(enActionNormalSkill));
	targetStateMachine->ActionButtonY(allowSkill && KeyConfig::Get().IsPress(enActionSpecialSkill));


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
	// ダッシュキーを押しているか
	bool isDashHolding = KeyConfig::Get().IsHolding(enActionDash);

	// スタミナが枯渇している場合はダッシュ入力をキャンセルする
	if (target_->IsExhausted())
	{
		isDashHolding = false;
	}

	// Aボタンをホールドしているか否か
	sm->ActionButtonA(isDashHolding);

	// 回避するか、ダッシュするか（回避はチュートリアルで解禁されるまで入力を無視する。ダッシュは常に許可）
	if (KeyConfig::Get().IsShortRelease(enActionDash))
	{
		const bool allowAvoid = TutorialManager::Get().IsActionAllowed(TutorialManager::Action::Avoid);
		if (allowAvoid && !target_->IsExhausted())
		{
			sm->SetAvoidRequested(true);
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
