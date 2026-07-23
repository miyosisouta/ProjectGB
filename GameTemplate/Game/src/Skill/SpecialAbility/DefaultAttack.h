#pragma once
#include "AbilityBase.h"
/**
 * 特殊能力未装備時に使うデフォルト攻撃クラス
 */
class DefaultAttack : public AbilityBase
{
public:
	/** コンストラクタ */
	DefaultAttack() {}
	/** デストラクタ */
	virtual ~DefaultAttack() {}

	/** スキル開始時に呼ぶ */
	void Enter(Character* p)override;
	/** 毎フレーム呼ぶ */
	void Update(Character* p)override;
	/** スキル終了時に呼ぶ */
	void Exit(Character* p)override;
};