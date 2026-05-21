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
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
	std::unique_ptr<GhostBody> attackHitbox_;
public:
	NormalAttackBase() {}
	virtual ~NormalAttackBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};


/*========================================*/
/** かみつきクラス */
/*========================================*/
class Bite : public NormalAttackBase
{
public:
	Bite() {}
	~Bite()override {}

	void Enter(Character* p)override;
	void Update(Character* p)override;
	void Exit(Character* p)override;
};