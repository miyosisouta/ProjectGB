#pragma once
/*
 * Stateの基底クラス
 */
class Character;
class IState : public Noncopyable
{	
protected:
	bool isFinished_ = false; //!< 現在のステート処理が終わったか
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
	std::unique_ptr<GhostBody> attackHitbox_;

public:
	/** 次の処理に行ってもよいか */
	virtual bool IsFinished() const { return isFinished_; }
	virtual bool IsCancelable() const { return false; }


public:
	IState() {}
	virtual ~IState() {}

	virtual void Enter() = 0;
	virtual void Update() = 0;
	virtual void Exit() = 0;
};