#pragma once
#include "IState.h"

class Player;
/*
 * Stateの基底クラス
 */
class PlayerStateBase: public IState
{	
protected:
	Player* player_ = nullptr; //!< プレイヤー

protected:
	/** 移動速度を計算する共通処理 */ 
	Vector3 CalcMovementVelocity(float speed);


public:
	/*
	 * コンストラクタ
	 * p : プレイヤーの情報を渡すため
	 */
	PlayerStateBase(Player* p) : player_(p) {}
	virtual ~PlayerStateBase() {}

	virtual void Enter() override= 0;
	virtual void Update() override = 0;
	virtual void Exit() override = 0;
};

/*==========================================*/
// 待機の状態
/*==========================================*/

class IdleState : public PlayerStateBase
{
public:
	IdleState(Player* p) : PlayerStateBase(p) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};

/*==========================================*/
// 歩きの状態
/*==========================================*/
class WalkState : public PlayerStateBase
{
public:
	WalkState(Player* p) : PlayerStateBase(p) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};

/*==========================================*/
// 走る状態
/*==========================================*/
class RunState : public PlayerStateBase
{
public:
	RunState(Player* p) : PlayerStateBase(p) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};

/************************************************************/
// ここからは強制的な状態のものを書く

/*==========================================*/
// 死亡の状態
/*==========================================*/

class DeadState : public PlayerStateBase
{
public:
	DeadState(Player* p) : PlayerStateBase(p) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};