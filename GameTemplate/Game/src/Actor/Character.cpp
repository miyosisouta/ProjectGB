#include "stdafx.h"
#include "Character.h"

namespace
{
	static Vector3 INIT_DIRECTION = Vector3(0.0f, 0.0f, 1.0f);
}


bool Character::Start()
{
	// ステートマシーンの初期化
	stateMachine_.InitialState(PlayerStateID::Idle);
	stateMachine_.SetDirection(INIT_DIRECTION);
	
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