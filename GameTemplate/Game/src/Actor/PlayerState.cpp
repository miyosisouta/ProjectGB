#include "stdafx.h"
#include "PlayerState.h"
#include "Player.h"

namespace
{
	const float WALK_BASE_SPEED = 100.0f;
	const float RUN_BASE_SPEED = 300.0f;
}

/*==================================================*/
/******************** 待機状態 **********************/
/*==================================================*/

void IdleState::Enter()
{
	// 待機アニメーション
	player_->PlayAnimation(StateID::Idle);
}

void IdleState::Update()
{
	// 移動速度はゼロに
	player_->SetMoveVelocity(Vector3::Zero);
}

void IdleState::Exit()
{
}


/*==================================================*/
/******************** 歩き状態 **********************/
/*==================================================*/

void WalkState::Enter()
{
	// 歩きアニメーショ
	player_->PlayAnimation(StateID::Walk);
}

void WalkState::Update()
{
	// 共通関数に歩く時の基本スピードを渡す
	Vector3 velocity = CalcMovementVelocity(WALK_BASE_SPEED);

	// 移動先をPlayerに渡す
	player_->SetMoveVelocity(velocity);
}

void WalkState::Exit()
{
}


/*==================================================*/
/******************** 走る状態 **********************/
/*==================================================*/

void RunState::Enter()
{
	// 走るアニメーション
	player_->PlayAnimation(StateID::Run);
}

void RunState::Update()
{
	// 共通関数に歩く時の基本スピードを渡す
	Vector3 velocity = CalcMovementVelocity(RUN_BASE_SPEED);

	// 移動先をPlayerに渡す
	player_->SetMoveVelocity(velocity);
}

void RunState::Exit()
{
}



void DeadState::Enter()
{
	// 死亡アニメーション
	player_->PlayAnimation(StateID::Dead);
}

void DeadState::Update()
{
	// 移動速度はゼロに
	player_->SetMoveVelocity(Vector3::Zero);
}

void DeadState::Exit()
{
}



/*=========================================*/
/** ここからは共通処理 */
/*=========================================*/



Vector3 PlayerStateBase::CalcMovementVelocity(float speed)
{
	// ステートマシーンを取得
	auto* stateMachine = player_->GetStateMachine();

	// 左スティックの入力量を取得
	float stickL = stateMachine->GetStickLAmount();

	// 入力がなければゼロベクトルを返す
	if (stickL < 0.01f) { return Vector3::Zero; }

	// 方向を取得
	Vector3 dir = stateMachine->GetDirection();

	// スピードとフレームレート(DeltaTime)を掛けて返す
	return dir * speed * g_gameTime->GetFrameDeltaTime();
}
