#include "stdafx.h"
#include "Character.h"


bool Character::Start()
{
	return Actor::Start();
}


void Character::Update()
{
	Actor::Update();
}


void Character::Render(RenderContext& rc)
{
	Actor::Render(rc);
}