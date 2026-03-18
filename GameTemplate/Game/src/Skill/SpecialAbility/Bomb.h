#pragma once
#include "AbilityBase.h"
/*
 * Bomb‚ÌŠî’êƒNƒ‰ƒX
 */
class BombBase : public AbilityBase
{	
public:
	BombBase() {}
	virtual ~BombBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};