#pragma once
#include"src/Skill/ISkill.h"
/**
 * 特殊能力の基底クラス
 */
class AbilityBase : public ISkill
{
protected:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_; //!< タイマー付きタスクの管理
	std::unique_ptr<GhostBody> attackHitbox_; //!< 攻撃判定用コリジョン

public:
	/** コンストラクタ */
	AbilityBase() {}
	/** デストラクタ */
	virtual ~AbilityBase() {}

	/** スキル開始時に呼ぶ */
	virtual void Enter(Character* p) = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update(Character* p) = 0;
	/** スキル終了時に呼ぶ */
	virtual void Exit(Character* p) = 0;
};