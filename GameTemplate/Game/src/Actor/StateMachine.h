#pragma once
#include "State.h"

/*
 * ステートマシーンの基底クラス
 */
class IStateMachine : public Noncopyable
{
protected:
	// std::unordered_map : キーと値を保存するためのコンテナ。ハッシュテーブルを使う。
	// ハッシュテーブル : 指定したIDのステートを検索する速度が非常に速い
	std::unordered_map<uint32_t, std::unique_ptr<IState>> m_stateMap; //!< 状態を管理するためのもの

	IState* m_currentState = nullptr; //!< 現在のステート 
	uint32_t m_currentStateId = INVALID_STATE_ID; //!< 現在のステートのID
	uint32_t m_nextStateId = INVALID_STATE_ID; //!< 次のステートのID


public:
	/* コンストラクタ */
	IStateMachine() {}
	/* デストラクタ */
	virtual ~IStateMachine() {}

	/* 更新処理 */
	virtual void Update();


protected:
	/* ステートの更新の共通処理 */
	void UpdateState();


public:
	/* 新しいステートの登録を行う */
	template <typename TState>
	void AddState()
	{
		m_stateMap.emplace(TState::ID(), std::make_unique<TState>());
	}

	/* 遷移先のステートのIDを登録 */
	template <typename TState>
	void InitialState()
	{
		m_nextStateId = TState::ID();
	}

	/* 現在のステートのIDを取得 */
	template <typename T>
	bool EqualsCurrentState() const
	{
		return m_currentStateId == T::ID();
	}


private:
	/* ステートの値を探索 */
	IState* FindState(const uint32_t id)
	{
		auto it = m_stateMap.find(id);
		if (it != m_stateMap.end()) {
			return it->second.get();
		}
		return nullptr;
	}
};
