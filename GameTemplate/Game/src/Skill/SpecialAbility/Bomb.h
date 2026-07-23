#pragma once
#include "AbilityBase.h"
/**
 * Bombの基底クラス
 */
class BombBase : public AbilityBase
{
private:
	ModelRender model_; //!< モデル

public:
	/** コンストラクタ */
	BombBase();
	/** デストラクタ */
	virtual ~BombBase()override;

	/** スキル開始時に呼ぶ */
	virtual void Enter(Character* p) = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update(Character* p) = 0;
	/** スキル終了時に呼ぶ */
	virtual void Exit(Character* p) = 0;
};

/** 地雷クラス */
class Landmine : public BombBase
{
public:
	/** コンストラクタ */
	Landmine();
	/** デストラクタ */
	~Landmine()override;

	/** スキル開始時に呼ぶ */
	void Enter(Character* p)override;
	/** 毎フレーム呼ぶ */
	void Update(Character* p)override;
	/** スキル終了時に呼ぶ */
	void Exit(Character* p)override;
};