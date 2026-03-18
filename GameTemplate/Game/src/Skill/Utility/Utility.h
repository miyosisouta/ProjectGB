#pragma once
#include"src/Skill/ISkill.h"
/*
 * “Áês“®‚ÌŠî’êƒNƒ‰ƒX
 */
class UtilityBase : public ISkill
{
protected:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
	std::unique_ptr<GhostBody> attackHitbox_;

public:
	UtilityBase() {}
	virtual ~UtilityBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};