#include "stdafx.h"
#include "PlayerState.h"
#include "Player.h"

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
}

void DeadState::Exit()
{
}
