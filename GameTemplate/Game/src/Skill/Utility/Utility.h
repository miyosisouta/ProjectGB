#pragma once
#include"src/Skill/ISkill.h"

/**
 * 特殊行動の基底クラス
 */
class UtilityBase : public ISkill
{
protected:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_; //!< タイマー付きタスクの管理
	std::unique_ptr<GhostBody> hitbox_; //!< 当たり判定用コリジョン

public:
	/** コンストラクタ */
	UtilityBase() {}
	/** デストラクタ */
	virtual ~UtilityBase() {}

	/** スキル開始時に呼ぶ */
	virtual void Enter(Character* p) = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update(Character* p) = 0;
	/** スキル終了時に呼ぶ */
	virtual void Exit(Character* p) = 0;
};


/* =========================================== */
/* 回避用クラス */
/* =========================================== */

class Avoid : public UtilityBase
{
private:
	Vector3 targetPos_ = Vector3::Zero; //!< 回避先の座標
	uint8_t avoidTriggerFrame_ = 0; //!< (未使用)回避判定フレーム

public:
	/** コンストラクタ */
	Avoid() {};
	/** デストラクタ */
	~Avoid()override {};

	/** スキル開始時に呼ぶ */
	void Enter(Character* p)override;
	/** 毎フレーム呼ぶ */
	void Update(Character* p) override;
	/** スキル終了時に呼ぶ */
	void Exit(Character* p)override;
};