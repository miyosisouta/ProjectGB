#pragma once
#include "IState.h"

class Player;
class NormalAttackBase;
class AbilityBase;
class UtilityBase;
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

	virtual bool IsFinished() const{ return false; }
	virtual bool IsCancelable() const { return false; }
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


/*==========================================*/
// 通常攻撃状態
/*==========================================*/
class NormalAttackState : public PlayerStateBase
{
private:
	NormalAttackBase* currentSkill_ = nullptr;

public:
	bool IsFinished() const;
	bool IsCancelable() const;

public:
	NormalAttackState(Player* p) : PlayerStateBase(p) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};

/*==========================================*/
// 特殊スキル状態
/*==========================================*/
class SpecialAbilityState : public PlayerStateBase
{
private:
	AbilityBase* currentSkill_ = nullptr;


public:
	bool IsFinished() const;
	bool IsCancelable() const;


public:
	SpecialAbilityState(Player* p) : PlayerStateBase(p) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};

/*==========================================*/
// 特殊行動状態
/*==========================================*/
class UtilityState : public PlayerStateBase
{
private:
	UtilityBase* currentSkill_ = nullptr;


public:
	bool IsFinished() const;
	bool IsCancelable() const;

public:
	UtilityState(Player* p) : PlayerStateBase(p) {}

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