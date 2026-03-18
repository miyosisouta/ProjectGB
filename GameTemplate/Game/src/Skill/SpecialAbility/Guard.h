#pragma once
#include "AbilityBase.h"
/*
 * Guard‚ÌŠî’êƒNƒ‰ƒX
 */
class GuardBase : public AbilityBase
{	
public:
	GuardBase() {}
	virtual ~GuardBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};