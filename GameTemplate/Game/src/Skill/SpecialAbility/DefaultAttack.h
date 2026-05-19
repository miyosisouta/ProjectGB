#pragma once
#include "AbilityBase.h"
/*
 * Bombの基底クラス
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