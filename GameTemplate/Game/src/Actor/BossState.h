#pragma once
#include "IState.h"
#include "src/Actor/Character.h"
#include "src/Util/Curve.h"
class BossCharacter;
class AttackRange;
class ThrowRockObject;
/**
 * Stateの基底クラス
 */
class BossStateBase: public IState
{
protected:
	BossCharacter* boss_; //!< 操作対象のボス


protected:
	/** プレイヤーに向けて移動速度を計算する共通処理 */
	Vector3 CalcMovePlayerVelocity(float speed);
	/** 指定した場所に向けて移動速度を計算する共通処理 */
	Vector3 CalcVelocityTowards(Vector3 targetPos, float speed);
	/** ターゲットに向かう回転の共通処理 */
	Quaternion RotateToTarget(float rotateSpeed);

	/**
	 * 攻撃前に「プレイヤーの方向へduration秒かけて振り向く」共通処理を開始する。
	 * 振り向きが完了すると onFinished を呼ぶ（攻撃本体の開始処理はonFinished側に書く）。
	 * HitStamp攻撃以外の攻撃ステートのEnter()から呼ぶ想定。
	 */
	void BeginTurnToPlayer(float duration, std::function<void()> onFinished);
	/**
	 * 振り向き処理の毎フレーム更新。
	 * 振り向き中はtrueを返すので、呼び出し元(各ステートのUpdate())はその場合攻撃側の更新処理をスキップする。
	 */
	bool UpdateTurnToPlayer();

	/**
	 * ボスの現在の攻撃速度倍率を取得する（EmotionSystem由来。1.0fが等倍）
	 * 攻撃系ステート（通常攻撃・ヒットスタンプ・回転・岩投げ・レーザー）が
	 * taskScheduler_->AddTimer() に渡す秒数をこの値で割ることで、攻撃の速さに反映する。
	 * Idle/Run/Death は「攻撃」ではないため使用しない。
	 * アニメーション・エフェクトの再生速度には使わない（GetAnimationSpeedMul/GetEffectSpeedMulを使う）。
	 */
	float GetAttackSpeedMul() const;
	/**
	 * ボスの現在のアニメーション再生速度倍率を取得する（EmotionSystem由来。1.0fが等倍）
	 * 攻撃速度(GetAttackSpeedMul)とは独立に調整できる演出用の値。boss_->PlayAnimation()に渡す。
	 */
	float GetAnimationSpeedMul() const;
	/**
	 * ボスの現在のエフェクト再生速度倍率を取得する（EmotionSystem由来。1.0fが等倍）
	 * 攻撃速度(GetAttackSpeedMul)とは独立に調整できる演出用の値。EffectManager::PlayEffect()に渡す。
	 */
	float GetEffectSpeedMul() const;


private:
	bool isTurningToPlayer_ = false;			//!< プレイヤー方向へ振り向き中か
	float turnElapsed_ = 0.0f;					//!< 振り向き開始からの経過時間
	float turnDuration_ = 1.0f;					//!< 振り向きにかける時間
	Quaternion turnStartRot_ = Quaternion::Identity;	//!< 振り向き開始時の回転
	Quaternion turnEndRot_ = Quaternion::Identity;		//!< 振り向き終了時（プレイヤー方向）の回転
	std::function<void()> onTurnFinished_;		//!< 振り向き完了時に呼ぶコールバック（攻撃開始処理）


public:
	/**
	 * コンストラクタ
	 * @param chara キャラクターの情報を渡すため
	 */
	BossStateBase(BossCharacter* chara) : boss_(chara) {}
	/** デストラクタ */
	virtual ~BossStateBase() {}

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

class BossIdleState : public BossStateBase
{
public:
	/** 処理が終わったか */
	inline bool IsFinished() const override { return isFinished_; }

public:
	/** コンストラクタ */
	BossIdleState(BossCharacter* b) : BossStateBase(b) {}

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
class BossRunState : public BossStateBase
{
private:
	float goalPos_ = 0.0f; //!< 目標座標

public:
	/** 処理が終わったか */
	inline bool IsFinished() const override{ return isFinished_; }

public:
	/** コンストラクタ */
	BossRunState(BossCharacter* b) : BossStateBase(b) {}

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
class BossAttackState : public BossStateBase
{
private:
	/** 振り向き終了後に呼ばれる、攻撃本体の開始処理 */
	void StartAttack();

public:
	/** 処理が終わったか */
	inline bool IsFinished() const override { return isFinished_; }

public:
	/** コンストラクタ */
	BossAttackState(BossCharacter* b) : BossStateBase(b) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
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
	Phase        phase_                = Phase::Ready; //!< 攻撃段階
	Vector3      targetPos_            = Vector3::Zero; //!< 移動先の座標
	Vector3      nextTargetPos_        = Vector3::Zero; //!< 次の移動先の座標
	Vector3      fixedAttackPos_       = Vector3::Zero; //!< 着地攻撃判定を固定する座標
	float        verticalVelocity_     = 0.0f;          //!< 垂直速度
	float        gravity_              = 0.0f;          //!< 重力
	bool         createAttackCollision_ = false;        //!< 攻撃用コリジョンを作成したかのフラグ
	bool         impactAnimPlayed_     = false;         //!< 着地インパクトアニメーション(enAnimJumpImpact)を再生済みか
	AttackRange* attackRange_          = nullptr;       //!< 攻撃予測インジケーター

public:
	/** 処理が終わりか */
	inline bool IsFinished() const override { return isFinished_; }

public:
	/** コンストラクタ */
	HitStampState(BossCharacter* b) : BossStateBase(b) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};


/*==========================================*/
// 回転攻撃
/*==========================================*/
class SpinState : public BossStateBase
{
private:
	Vector3      targetPos_              = Vector3::Zero;       //!< 攻撃対象の座標
	bool         isAttackStart_          = false;               //!< 攻撃をスタートするか
	EffectHandle spinEffectHandle_       = INVALID_EFFECT_HANDLE; //!< 回転エフェクトのハンドル
	AttackRange* attackRangeIndicator_   = nullptr;             //!< 攻撃予測ラインインジケーター
	float        predictionElapsed_      = 0.0f;                //!< 予測表示の経過時間 (drawProgress 計算用)
	FloatCurve   jumpCurve_;                                    //!< 突進開始直前のジャンプ演出（Y座標の上下）を管理するカーブ

private:
	/** 振り向き終了後に呼ばれる、攻撃本体の開始処理 */
	void StartAttack();

public:
	/** 処理が終わりか */
	inline bool IsFinished() const override { return isFinished_; }

public:
	/** コンストラクタ */
	SpinState(BossCharacter* b) : BossStateBase(b) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
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
	Phase             phase_                = Phase::enReady;        //!< 攻撃段階
	Vector3           targetPos_            = Vector3::Zero;         //!< 攻撃対象の座標
	EffectHandle      predictionEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 予測エフェクトのハンドル
	AttackRange*      attackRangeIndicator_ = nullptr;               //!< 攻撃予測ラインインジケーター
	float             predictionElapsed_    = 0.0f;                  //!< 予測表示の経過時間
	FloatCurve        windUpSwayCurve_;                              //!< ワインドアップ中に左右へ揺さぶる角度(度)のイージング（PingPongで往復し続ける）
	Quaternion        windUpSwayBaseRot_    = Quaternion::Identity;  //!< 揺さぶりの基準になる向き（プレイヤー方向を向いた状態）
	ThrowRockObject*  pendingRock_          = nullptr;               //!< ワインドアップ中に出現させ、Launch()前まで成長させ続けている岩（予測ラインと同じ進捗値で駆動する）

private:
	/** 振り向き終了後に呼ばれる、攻撃本体の開始処理 */
	void StartAttack();

public:
	/** 処理が終わりか */
	inline bool IsFinished() const override { return isFinished_; }


public:
	/** コンストラクタ */
	ThrowRockState(BossCharacter* b) : BossStateBase(b) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};



/*==========================================*/
// レーザー攻撃
/*==========================================*/
class LaserState : public BossStateBase
{
private:
	/** 処理段階 */
	enum Phase
	{
		enReady,	//!< 準備
		enPredict,	//!< 予測
		enShot,		//!< 発射
		enDone,		//!< 何もなし
	};

	/** 攻撃パターン */
	enum Mode
	{
		enNormal,	//!< クイック攻撃
		enMult,		//!< 連発する攻撃
		enCharge,	//!< チャージ攻撃
		enMax,		//!< 攻撃手段の数
		enWeightMax = 10 //!< 重みの合計
	};


private:
	Phase        phase_                = Phase::enReady;        //!< 処理段階
	Mode         mode_                 = Mode::enMax;           //!< 攻撃パターン
	uint8_t      weights_[enMax]       = { 3,4,3 };             //!< 攻撃の重み
	Vector3      targetPos_            = Vector3::Zero;         //!< 攻撃対象の座標
	EffectHandle laserEffectHandle_    = INVALID_EFFECT_HANDLE; //!< レーザーエフェクトのハンドル
	AttackRange* attackRangeIndicator_ = nullptr;               //!< 攻撃予測サークルインジケーター


	Vector3 scale_          = Vector3::Zero; //!< 攻撃範囲
	float   shotTime_       = 0.0f;          //!< タスクスケジューラの次の処理までの時間
	float   attackDeleyTime = 0.0f;          //!< 攻撃までの時間
	uint8_t shotCount_      = 0;             //!< 攻撃回数
	float   attackSpeedMul_ = 1.0f;          //!< Enter()冒頭で取得する攻撃速度倍率。Setup()内の[timing]計算で使う

	FloatCurve   multiJumpCurve_;      //!< 連発攻撃前の予備動作（上下ジャンプ）のY座標を管理するカーブ
	float        multiJumpElapsed_ = 0.0f; //!< 予備ジャンプ開始からの経過時間（既定回数繰り返したか判定するため）
	Vector3Curve chargeScaleCurve_;    //!< チャージ攻撃前後のScale変化（通常Scale→拡大→攻撃→通常Scaleへ復帰）を管理するカーブ
	Vector3      normalScale_ = Vector3::One; //!< Enter()時点のボスの通常Scale（CharacterStatusData.json由来）。チャージ演出の拡大前/復帰先として使う
	bool         isChargeScaleReturning_ = false; //!< chargeScaleCurve_が「攻撃前の拡大」ではなく「攻撃後に通常Scaleへ戻す」動作を再生中か（phase_は前回の攻撃で使い回され既にenDoneになっているため、これで判定する）


private:
	/** セットアップ */
	void Setup();
	/** 振り向き終了後に呼ばれる、攻撃タイプに応じた予備動作の開始処理（連発は上下ジャンプ後にStartAttack()を呼ぶ。通常/チャージはそのままStartAttack()を呼ぶ。チャージのScale演出はEnter()側で振り向きと並行して開始済み） */
	void BeginPreMotion();
	/** 予備動作終了後に呼ばれる、攻撃本体の開始処理 */
	void StartAttack();


public:
	/** 攻撃終了か */
	inline bool IsFinished() const override { return isFinished_; }


public:
	/** コンストラクタ */
	LaserState(BossCharacter* b) : BossStateBase(b) {}

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

class BossDeathState : public BossStateBase
{
public:
	/** コンストラクタ */
	BossDeathState(BossCharacter* b) : BossStateBase(b) {}

	/** ステート開始時に呼ぶ */
	void Enter()override;
	/** 毎フレーム呼ぶ */
	void Update()override;
	/** ステート終了時に呼ぶ */
	void Exit()override;
};