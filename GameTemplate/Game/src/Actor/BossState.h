#pragma once
#include "IState.h"

class BossCharacter;
/*
 * Stateの基底クラス
 */
class BossStateBase: public IState
{	
protected:
	BossCharacter* boss_ = nullptr; //!< プレイヤー


	/** 移動速度を計算する共通処理 */
	Vector3 CalcMovementVelocity(float speed);
	/** ターゲットに向かう回転の共通処理 */
	Quaternion RotateToTarget(float rotateSpeed);


public:
	/*
	 * コンストラクタ
	 * p : プレイヤーの情報を渡すため
	 */
	BossStateBase(BossCharacter* b) : boss_(b) {}
	virtual ~BossStateBase() {}

	virtual void Enter() override= 0;
	virtual void Update() override = 0;
	virtual void Exit() override = 0;

	virtual bool IsFinished() const{ return false; }
	virtual bool IsCancelable() const { return false; }
};

/*==========================================*/
// 待機の状態
/*==========================================*/

class BossIdleState : public BossStateBase
{
public:
	bool IsFinished() const override { return isFinished; }

public:
	BossIdleState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};


/*==========================================*/
// 走る状態
/*==========================================*/
class BossRunState : public BossStateBase
{
private:
	float goalPos_ = 0.0f;

public:
	bool IsFinished() const override{ return isFinished; }

public:
	BossRunState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};



/************************************************************/
// ここからは強制的な状態のものを書く

/*==========================================*/
// 死亡の状態
/*==========================================*/

class BossDeathState : public BossStateBase
{
public:
	BossDeathState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};