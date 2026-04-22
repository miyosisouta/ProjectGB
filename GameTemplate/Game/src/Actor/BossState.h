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


protected:
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
	/* 処理が終わったか */
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
	/* 目標座標 */
	float goalPos_ = 0.0f;

public:
	/* 処理が終わったか */
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
	/* 処理が終わったか */
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
		Ready,			//!< 準備中
		JumpUp,			//!< 上昇中
		Hover,			//!< 空中待機中
		FallDown,		//!< 急降下中
		ShokingStamp,	//!< 着地時
		Finished		//!< 着地後
	};

private:
	Phase phase_ = Phase::Ready; //!< 攻撃段階
	Vector3 targetPos_ = Vector3::Zero; //!< 移動先の座標
	Vector3 nextTargetPos_ = Vector3::Zero; //!< 次の移動先の座標
	Vector3 fixedAttackPos_ = Vector3::Zero;
	float verticalVelocity_ = 0.0f;  //!< 垂直速度
	float gravity_ = 0.0f;        //!< 重力
	bool createAttackCollision_ = false; //!< 攻撃用コリジョンを作成したかのフラグ
	EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 攻撃予測エフェクトのハンドル
	
public:
	/* 処理が終わりか */
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
	Vector3 targetPos_ = Vector3::Zero; //!< 攻撃対象の座標
	bool isAttackStart_ = false; //!< 攻撃をスタートするか
	EffectHandle spinEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 回転エフェクトのハンドル
	EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 攻撃予測用エフェクトのオハンドル

public:
	/* 処理が終わりか */
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
		enReady,	//!< 準備
		enPredict,	//!< 予測
		enThrow,	//!< 攻撃
		enDone,		//!< 何もなし
	};

private:
	Phase phase_ = Phase::enReady; //!< 攻撃段階
	Vector3 targetPos_ = Vector3::Zero; //!< 攻撃対象の座標
	EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE; // 予測エフェクトのハンドル

public:
	/* 処理が終わりか */
	bool IsFinished() const override { return isFinished_; }


public:
	ThrowRockState(BossCharacter* b) : BossStateBase(b) {}

	void Enter()override;
	void Update()override;
	void Exit()override;
};



/*==========================================*/
// レーザー攻撃
/*==========================================*/
class LaserState : public BossStateBase
{
private:
	/* 処理段階 */
	enum Phase
	{
		enReady,	//!< 準備
		enPredict,	//!< 予測
		enShot,		//!< 発射
		enDone,		//!< 何もなし
	};

	/* 攻撃パターン */
	enum Mode 
	{
		enNormal,	//!< クイック攻撃
		enMult,		//!< 連発する攻撃
		enCharge,	//!< チャージ攻撃
		enMax,		//!< 攻撃手段の数
		enWeightMax = 10 //!< 重みの合計
	};


private:
	Phase phase_ = Phase::enReady; //!< 処理段階
	Mode mode_ = Mode::enMax; //!< 攻撃パターン
	uint8_t weights_[enMax] = { 4,3,3 }; //!< 攻撃の重み
	Vector3 targetPos_ = Vector3::Zero; //!< 攻撃対象の座標
	EffectHandle predictionEffectHandle_= INVALID_EFFECT_HANDLE; //!< 予測エフェクトのハンドル
	EffectHandle laserEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 予測エフェクトのハンドル


	Vector3 scale_ = Vector3::Zero; //!< 攻撃範囲
	float shotTime_ = 0.0f; //!< タスクスケジューラの次の処理までの時間
	float attackDeleyTime = 0.0f; //!< 攻撃までの時間
	uint8_t shotCount_ = 0; //!< 攻撃回数

	
private:
	/* セットアップ */
	void Setup();


public:
	/* 攻撃終了か */
	bool IsFinished() const override { return isFinished_; }


public:
	LaserState(BossCharacter* b) : BossStateBase(b) {}

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