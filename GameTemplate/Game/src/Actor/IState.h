#pragma once
/*
 * Stateの基底クラス
 */
class IState : public Noncopyable
{	
protected:
	bool isFinished = false; //!< 現在のステート処理が終わったか
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_;


public:
	/** 次の処理に行ってもよいか */
	virtual bool IsFinished() const { return isFinished; }
	virtual bool IsCancelable() const { return false; }


public:
	IState() {}
	virtual ~IState() {}

	virtual void Enter() = 0;
	virtual void Update() = 0;
	virtual void Exit() = 0;
};