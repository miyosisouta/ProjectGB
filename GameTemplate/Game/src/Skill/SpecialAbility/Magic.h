#pragma once
#include "AbilityBase.h"
/*
 * Magicの基底クラス
 */
class MagicBase : public AbilityBase
{	
public:
	MagicBase();
	virtual ~MagicBase();

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};

class FireMagic : public MagicBase
{
private:
	EffectHandle fireEffectHandle_ = INVALID_EFFECT_HANDLE; // 予測エフェクトのハンドル

public:
	FireMagic();
	~FireMagic()override;

	void Enter(Character* p)override;
	void Update(Character* p)override;
	void Exit(Character* p)override;
};