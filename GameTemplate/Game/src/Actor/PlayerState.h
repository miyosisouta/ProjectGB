#pragma once
#include "IState.h"

class Player;
class NormalAttackBase;
class AbilityBase;
class UtilityBase;
/**
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
	/** コンストラクタ */
	PlayerStateBase(Player* p) : player_(p) {}
	/** デストラクタ */
	virtual ~PlayerStateBase() {}

	/** ステート開始時に呼ぶ */
	virtual void Enter() override= 0;
	/** 毎フレーム呼ぶ */
	virtual void Update() override = 0;
	/** ステート終了時に呼ぶ */
	virtual void Exit() override = 0;

	/** 処理が終わったか */
	inline virtual bool IsFinished() const{ return false; }
	/** 他のステートに割り込みキャンセル可能か */
	inline virtual bool IsCancelable() const { return false; }
};

/*==========================================*/
// 待機の状態
/*==========================================*/

class IdleState : public PlayerStateBase
{
public:
	/** コンストラクタ */
	IdleState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};

/*==========================================*/
// 歩きの状態
/*==========================================*/
class WalkState : public PlayerStateBase
{
private:
	float walkSEElapsedTime_ = 0.0f; //!< 歩きのSEを再生した経過時間
	float baseMoveSpeed_ = 0.0f;	 //!< 歩く速度のベース値

public:
	/** コンストラクタ */
	WalkState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};

/*==========================================*/
// 走る状態
/*==========================================*/
class RunState : public PlayerStateBase
{
private:
	float runSEElapsedTime_ = 0.0f; //!< 走りのSEを再生した経過時間


public:
	/** コンストラクタ */
	RunState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};


/*==========================================*/
// 通常攻撃状態
/*==========================================*/
class NormalAttackState : public PlayerStateBase
{
private:
	NormalAttackBase* currentSkill_ = nullptr; //!< 現在装備している通常攻撃スキル

public:
	/** 処理が終わったか */
	bool IsFinished() const;
	/** 他のステートに割り込みキャンセル可能か */
	bool IsCancelable() const;

public:
	/** コンストラクタ */
	NormalAttackState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};

/*==========================================*/
// 特殊スキル状態
/*==========================================*/
class SpecialAbilityState : public PlayerStateBase
{
private:
	AbilityBase* currentSkill_ = nullptr; //!< 現在装備している特殊能力スキル


public:
	/** 処理が終わったか */
	bool IsFinished() const;
	/** 他のステートに割り込みキャンセル可能か */
	bool IsCancelable() const;


public:
	/** コンストラクタ */
	SpecialAbilityState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};

/*==========================================*/
// 特殊行動状態
/*==========================================*/
class UtilityState : public PlayerStateBase
{
private:
	UtilityBase* currentSkill_ = nullptr; //!< 現在装備している汎用スキル


public:
	/** 処理が終わったか */
	bool IsFinished() const;
	/** 他のステートに割り込みキャンセル可能か */
	bool IsCancelable() const;

public:
	/** コンストラクタ */
	UtilityState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};


/************************************************************/
// ここからは強制的な状態のものを書く

/*==========================================*/
// 死亡の状態
/*==========================================*/

class DeathState : public PlayerStateBase
{
public:
	/** コンストラクタ */
	DeathState(Player* p) : PlayerStateBase(p) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};