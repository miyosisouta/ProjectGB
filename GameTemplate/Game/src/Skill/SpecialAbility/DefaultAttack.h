#pragma once
#include "AbilityBase.h"
/*
 * Bomb‚ÌŠî’êƒNƒ‰ƒX
 */
class DefaultAttack : public AbilityBase
{	
public:
	DefaultAttack() {}
	virtual ~DefaultAttack() {}

	void Enter(Character* p)override;
	void Update(Character* p)override;
	void Exit(Character* p)override;
};