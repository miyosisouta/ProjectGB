#pragma once
#include "src/Skill/ISkill.h"

class TaskSchedulerSystem;
class GhostBody;
 /*========================================*/
 /** 通常攻撃の基底クラス */
 /*========================================*/
class NormalAttackBase : public ISkill
{
protected:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_; //!< タイマー付きタスクの管理
	std::unique_ptr<GhostBody> attackHitbox_; //!< 攻撃判定用コリジョン
public:
	/** コンストラクタ */
	NormalAttackBase() {}
	/** デストラクタ */
	virtual ~NormalAttackBase() {}

	/** スキル開始時に呼ぶ */
	virtual void Enter(Character* p) = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update(Character* p) = 0;
	/** スキル終了時に呼ぶ */
	virtual void Exit(Character* p) = 0;
};


/*========================================*/
/** かみつきクラス */
/*========================================*/
class Bite : public NormalAttackBase
{
public:
	/** コンストラクタ */
	Bite() {}
	/** デストラクタ */
	~Bite()override {}

	/** スキル開始時に呼ぶ */
	void Enter(Character* p)override;
	/** 毎フレーム呼ぶ */
	void Update(Character* p)override;
	/** スキル終了時に呼ぶ */
	void Exit(Character* p)override;
};