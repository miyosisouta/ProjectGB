#pragma once
#include "IState.h"
#include "src/Actor/Character.h"
class BossCharacter;
/*
 * Stateの基底クラス
 */
class BossStateBase: public IState
{	
protected:
	BossCharacter* boss_; //!< プレイヤー


	/** プレイヤーに向けて移動速度を計算する共通処理 */
	Vector3 CalcMovePlayerVelocity(float speed);
	/** 指定した場所に向けて移動速度を計算する共通処理 */
	Vector3 CalcVelocityTowards(Vector3 targetPos, float speed);
	/** ターゲットに向かう回転の共通処理 */
	Quaternion RotateToTarget(float rotateSpeed);


public:
	/*
	 * コンストラクタ
	 * chara : キャラクターの情報を渡すため
	 */
	BossStateBase(BossCharacter* chara) : boss_(chara) {}
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
	bool IsFinished() const override { return isFinished_; }

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
	bool IsFinished() const override{ return isFinished_; }

public:
	BossRunState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};


/*==========================================*/
// 通常攻撃状態
/*==========================================*/
class BossAttackState : public BossStateBase
{
public:
	bool IsFinished() const override { return isFinished_; }

public:
	BossAttackState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};



/*==========================================*/
// ヒットスタンプ攻撃
/*==========================================*/
class HitStampState : public BossStateBase
{
private:
	enum Phase
	{
		Ready,			// 準備中
		JumpUp,			// 上昇中
		Hover,			// 空中待機中
		FallDown,		// 急降下中
		ShokingStamp,	// 着地時
		Finished		// 着地後
	};

private:
	Phase phase_ = Phase::Ready;
	Vector3 targetPos_ = Vector3::Zero;
	Vector3 nextTargetPos_ = Vector3::Zero;
	Vector3 fixedAttackPos_ = Vector3::Zero;
	float verticalVelocity_ = 0.0f;  // 垂直速度
	float gravity_ = 0.0f;        // 重力
	bool createAttackCollision_ = false;
	EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 攻撃予測エフェクトのハンドル（追従管理用）
	
public:
	bool IsFinished() const override { return isFinished_; }

public:
	HitStampState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};


/*==========================================*/
// 回転攻撃
/*==========================================*/
class SpinState : public BossStateBase
{
private:
	Vector3 targetPos_ = Vector3::Zero;
	bool isAttackStart_ = false;
	EffectHandle spinEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 回転エフェクトのハンドル（追従管理用）
	EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE;

public:
	bool IsFinished() const override { return isFinished_; }

public:
	SpinState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};


/*==========================================*/
// 岩を投げる攻撃
/*==========================================*/
class ThrowRockState : public BossStateBase
{
private:
	enum Phase
	{
		enReady,
		enPredict,
		enThrow,
		enDone,
	};

private:
	Phase phase_ = Phase::enReady;
	Vector3 targetPos_ = Vector3::Zero;
	EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE; // 予測エフェクトのハンドル

public:
	bool IsFinished() const override { return isFinished_; }


public:
	ThrowRockState(BossCharacter* b) : BossStateBase(b) {}

	void Render(RenderContext& rc);

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