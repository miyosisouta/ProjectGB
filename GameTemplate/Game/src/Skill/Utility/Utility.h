#pragma once
#include"src/Skill/ISkill.h"

/*
 * 特殊行動の基底クラス
 */
class UtilityBase : public ISkill
{
protected:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
	std::unique_ptr<GhostBody> hitbox_;

public:
	UtilityBase() {}
	virtual ~UtilityBase() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};


/* =========================================== */
/* 回避用クラス */
/* =========================================== */

class Avoid : public UtilityBase
{
private:
	Vector3 targetPos_ = Vector3::Zero;
	uint8_t avoidTriggerFrame_ = 0;

public:
	Avoid() {};
	~Avoid()override {};

	void Enter(Character* p)override;
	void Update(Character* p) override;
	void Exit(Character* p)override;
};