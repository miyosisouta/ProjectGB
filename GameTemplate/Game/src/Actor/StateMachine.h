#pragma once
#include "IState.h"

/** プレイヤーの状態 */
enum class PlayerStateID
{
	/* 基本ステート */
	None,
	Idle,	//!< 待機
	Walk,	//!< 歩く
	Run,	//!< 走る

	/* 通常攻撃 */
	Bite,	//!< 通常攻撃

	/* スキル攻撃 */
	DefaultAttack,	//!< デフォルトスキル攻撃
	Landmine,		//!< 地雷攻撃
	FireMagic,		//!< 火魔法攻撃

	/* 汎用スキル */
	Avoid, //!< 回避

	/* 強制ステート */
	Dead
};

/** 状態遷移に関する変数の構造体 */
struct Transition
{
	std::function<bool()> condition; //!< 遷移する条件
	PlayerStateID nextState;		 //!< 条件を満たした時の遷移先
};


/**
 * 汎用型ステートマシーン(継承せずに実体として使う)
 */
class StateMachine : public Noncopyable
{
private:
	/**
	 * std::unordered_map : キーと値を保存するためのコンテナ。ハッシュテーブルを使う。
	 * ハッシュテーブル : 指定したIDのステートを検索する速度が非常に速い
	 */
	std::unordered_map<PlayerStateID, std::unique_ptr<IState>> stateMap_; //!< 状態を管理するためのもの

	/* 遷移ルールのリストを追加 */
	std::vector<Transition> globalTransitions_;										//!< 優先される遷移ルールのリスト(例:死ぬフラグ,ダメージリアクション)
	std::unordered_map<PlayerStateID, std::vector<Transition>> stateTransitions_;	//!< 遷移ルールのリスト(例:待機,歩き,走り)

	/* 状態のステート */
	IState* currentState_ = nullptr;					 //!< 現在のステート
	PlayerStateID currentStateId_ = PlayerStateID::None; //!< 現在のステートのid
	PlayerStateID nextStateId_ = PlayerStateID::None;	 //!< 次のステートのid

private:
	Quaternion rotation_ = Quaternion::Identity; //!< 回転
	Vector3 direction_ = Vector3::Zero;			 //!< 方向
	float stickLAmount_ = 0.0f;		//!< 左スティックの入力量
	bool actionButtonA_ = false;	//!< Aボタンを長押しでダッシュするか、短押しで回避するか
	bool actionButtonB_ = false;	//!< Bボタンを押したか
	bool actionButtonX_ = false;	//!< Xボタンを押したか
	bool actionButtonY_ = false;	//!< Yボタンを押したか
	bool avoidRequested_ = false;	//!< 回避をするか


private:
	/** ステートを見つける */
	IState* FindState(const PlayerStateID id)
	{
		auto it = stateMap_.find(id);
		if (it != stateMap_.end()) {
			return it->second.get();
		}
		return nullptr;
	}


public:
	/** 回転の取得 */
	inline Quaternion GetRotation() { return rotation_; }
	/** 回転の設定 */
	inline void SetRotation(const Quaternion rot) { rotation_ = rot; }
	/** 方向の取得 */
	inline const Vector3& GetDirection() { return direction_; }
	/** 方向の設定 */
	inline void SetDirection(const Vector3& dir) { direction_ = dir; }
	/** 入力されているか */
	inline float GetStickLAmount() { return stickLAmount_; }
	/** 入力量を設定 */
	inline void SetStickLAmount(const float stickLAmount) { stickLAmount_ = stickLAmount; }
	/** Aボタンされているか */
	inline bool IsDash() { return actionButtonA_; }
	/** Aボタンの設定 */
	inline void ActionButtonA(const bool flg) { actionButtonA_ = flg; }
	/** Bボタンされているか */
	inline bool IsActionButtonB() { return actionButtonB_; }
	/** Bボタンの設定 */
	inline void ActionButtonB(const bool flg) { actionButtonB_ = flg; }
	/** Yボタンされているか */
	inline bool IsActionButtonY() { return actionButtonY_; }
	/** Yボタンの設定 */
	inline void ActionButtonY(const bool flg) { actionButtonY_ = flg; }
	/** Xボタンされているか */
	inline bool IsActionButtonX() { return actionButtonX_; }
	/** Xボタンの設定 */
	inline void ActionButtonX(const bool flg) { actionButtonX_ = flg; }


	/** 現在のステートを取得 */
	inline IState* GetCurrentState() const{ return currentState_; }
	/** 現在のステートIDを取得 */
	inline PlayerStateID GetCurrentStateId() const { return currentStateId_; }

	/** 回避ステート実行中は新たな回避入力を受け付けない */
	inline void SetAvoidRequested(bool flg) {
		if (flg && currentStateId_ == PlayerStateID::Avoid) { return; }
		avoidRequested_ = flg;
	}
	/** 回避が要求されているか */
	inline bool IsAvoidRequested() { return avoidRequested_; }
	/** 回避要求をクリア */
	inline void ClearAvoidRequest() { avoidRequested_ = false; }

	/** 入力処理をすべてリセット */
	void ResetInput()
	{
		SetStickLAmount(0.0f);
		SetDirection(Vector3::Zero);
		ActionButtonA(false);
		ActionButtonB(false);
		ActionButtonX(false);
		ActionButtonY(false);
		SetAvoidRequested(false);
	}


public:
	/** コンストラクタ */
	StateMachine() {}
	/** デストラクタ */
	virtual ~StateMachine() = default;

	/** 更新処理 */
	virtual void Update();
	/** ステートの切り替えと更新 */
	void UpdateState();

/*================================================================*/
/** ステート関係 */
/*================================================================*/

public:
	/** 新しいステートの登録を行う */
	inline void AddState(PlayerStateID id,IState* state)
	{
		stateMap_.emplace(id, std::unique_ptr<IState>(state));
	}

	/** IDをセット */
	inline void InitialState(PlayerStateID id)
	{
		nextStateId_ = id;
	}

	/**
	 * どの状態からでも遷移できる「グローバル遷移ルール」を登録
	 * 現在のアクションを強制キャンセル、割り込むことができる
	 * 例：死亡時、ダメージリアクションなど
	 * @param condition 遷移条件を満たしたかを判定するコールバック関数（trueを返すと遷移を実行）
	 * @param nextState 条件を満たした際の遷移先のステートID
	 */
	inline void AddGlobalTransition(std::function<bool()> condition, PlayerStateID nextState)
	{
		globalTransitions_.push_back({ condition, nextState });
	}

	/**
	 * 特定のステートにいる時のみ有効になる「個別の遷移ルール」を登録
	 * グローバル遷移ルールの条件を満たしていない場合のみチェックされます。
	 * @param state 遷移前のステートID
	 * @param nextState 遷移先のステートID,
	 * @param condition 遷移条件を満たしたかを判定するコールバック関数（trueを返すと遷移を実行）
	 */
	inline void AddTransition(PlayerStateID state, PlayerStateID nextState, std::function<bool()> condition)
	{
		stateTransitions_[state].push_back({ condition, nextState });
	}
};