#pragma once
#include"src/Skill/ISkill.h"
/*
 * 特殊能力の基底クラス
 */
class AbilityBase : public ISkill
{
protected:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
	std::unique_ptr<GhostBody> attackHitbox_;

public:
	AbilityBase() {}
	virtual ~AbilityBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};