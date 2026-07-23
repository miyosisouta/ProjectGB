#pragma once
#include "AbilityBase.h"
/**
 * Magicの基底クラス
 */
class MagicBase : public AbilityBase
{
public:
	/** コンストラクタ */
	MagicBase();
	/** デストラクタ */
	virtual ~MagicBase();

	/** スキル開始時に呼ぶ */
	virtual void Enter(Character* p) = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update(Character* p) = 0;
	/** スキル終了時に呼ぶ */
	virtual void Exit(Character* p) = 0;
};

/** 火魔法クラス */
class FireMagic : public MagicBase
{
private:
	EffectHandle fireEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 炎エフェクトのハンドル

public:
	/** コンストラクタ */
	FireMagic();
	/** デストラクタ */
	~FireMagic()override;

	/** スキル開始時に呼ぶ */
	void Enter(Character* p)override;
	/** 毎フレーム呼ぶ */
	void Update(Character* p)override;
	/** スキル終了時に呼ぶ */
	void Exit(Character* p)override;
};