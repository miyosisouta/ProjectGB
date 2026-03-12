#pragma once
#include "IState.h"

/* プレイヤーの状態 */
enum class StateID
{
	None,
	Idle,
	Walk,
	Run,
	Jump,
	Dead
};

// 状態遷移に関する変数の構造体
struct Transition
{
	std::function<bool()> condition; //!< 遷移する条件
	StateID nextState;               //!< 条件を満たした時の遷移先
};


/*
 * 汎用型ステートマシーン(継承せずに実体として使う)
 */
class StateMachine : public Noncopyable
{
private:
	/*
	 * std::unordered_map : キーと値を保存するためのコンテナ。ハッシュテーブルを使う。
	 * ハッシュテーブル : 指定したIDのステートを検索する速度が非常に速い
	 */ 
	std::unordered_map<StateID, std::unique_ptr<IState>> stateMap_; //!< 状態を管理するためのもの

	/* 遷移ルールのリストを追加 */
	std::vector<Transition> globalTransitions_;								//!< 優先される遷移ルールのリスト(例:死ぬフラグ,ダメージリアクション)
	std::unordered_map<StateID, std::vector<Transition>> stateTransitions_;	//!< 遷移ルールのリスト(例:待機,歩き,走り)

	/* 状態のステート */
	IState* currentState_ = nullptr;		 //!< 現在のステート
	StateID currentStateId_ = StateID::None; //!< 現在のステートのid
	StateID nextStateId_ = StateID::None;	 //!< 次のステートのid


public:
	/* コンストラクタ */
	StateMachine() {}
	/* デストラクタ */
	virtual ~StateMachine() = default;

	/* 更新処理 */
	virtual void Update();
	/* ステートの切り替えと更新 */
	void UpdateState();


/************************** ステート関係 **************************/

public:
	/* 新しいステートの登録を行う */
	void AddState(StateID id,IState* state)
	{
		stateMap_.emplace(id, std::unique_ptr<IState>(state));
	}

	/* IDをセット */
	void InitialState(StateID id)
	{
		nextStateId_ = id;
	}

	/* 
	 * どの状態からでも遷移できる「グローバル遷移ルール」を登録 
	 * 現在のアクションを強制キャンセル、割り込むことができる
	 * 例：死亡時、ダメージリアクションなど
	 * @param condition 遷移条件を満たしたかを判定するコールバック関数（trueを返すと遷移を実行）
	 * @param nextState 条件を満たした際の遷移先のステートID
	 */
	void AddGlobalTransition(std::function<bool()> condition, StateID nextState)
	{
		globalTransitions_.push_back({ condition, nextState });
	}

	/* 
	 * 特定のステートにいる時のみ有効になる「個別の遷移ルール」を登録 
	 * グローバル遷移ルールの条件を満たしていない場合のみチェックされます。
	 * @param state 遷移前のステートID
	 * @param nextState 遷移先のステートID, 
	 * @param condition 遷移条件を満たしたかを判定するコールバック関数（trueを返すと遷移を実行）
	 */
	void AddTransition(StateID state, StateID nextState, std::function<bool()> condition)
	{
		stateTransitions_[state].push_back({ condition, nextState });
	}

private:
	/* ステートを見つける */ 
	IState* FindState(const StateID id)
	{
		auto it = stateMap_.find(id);
		if (it != stateMap_.end()) {
			return it->second.get();
		}
		return nullptr;
	}
};