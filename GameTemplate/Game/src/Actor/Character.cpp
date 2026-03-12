#include "stdafx.h"
#include "Character.h"


bool Character::Start()
{
	// ステートマシーンの初期化(待機)
	stateMachine_.InitialState(StateID::Idle);
	
	return Actor::Start();
}


void Character::Update()
{
	Actor::Update();

	// ステートマシンの更新
	stateMachine_.Update();
}


void Character::Render(RenderContext& rc)
{
	Actor::Render(rc);
}