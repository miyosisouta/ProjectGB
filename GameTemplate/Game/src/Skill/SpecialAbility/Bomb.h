#pragma once
#include "AbilityBase.h"
/*
 * Bomb‚ÌŠî’êƒNƒ‰ƒX
 */
class BombBase : public AbilityBase
{	
public:
	BombBase();
	virtual ~BombBase()override;

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};

class Landmine : public BombBase
{
private:
	ModelRender model_;


public:
	Landmine();
	~Landmine()override;

	void Enter(Character* p)override;
	void Update(Character* p)override;
	void Exit(Character* p)override;
};