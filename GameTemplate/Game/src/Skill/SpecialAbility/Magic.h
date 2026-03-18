#pragma once
#include "AbilityBase.h"
/*
 * Magic‚ÌŠî’êƒNƒ‰ƒX
 */
class MagicBase : public AbilityBase
{	
public:
	MagicBase() {}
	virtual ~MagicBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};