#pragma once
#include "State.h"

class IStateMachine : public Noncopyable
{
protected:
	std::unordered_map<uint32_t, std::unique_ptr<IState>> m_stateMap;
	IState* m_currentState = nullptr;
	uint32_t m_currentStateId = INVALID_STATE_ID;
	uint32_t m_nextStateId = INVALID_STATE_ID;


public:
	IStateMachine() {}
	virtual ~IStateMachine() {}


	virtual void Update();


protected:
	void UpdateState();


public:
	template <typename TState>
	void AddState()
	{
		m_stateMap.emplace(TState::ID(), std::make_unique<TState>());
	}

	template <typename TState>
	void InitialState()
	{
		m_nextStateId = TState::ID();
	}


	template <typename T>
	bool EqualsCurrentState() const
	{
		return m_currentStateId == T::ID();
	}


private:
	IState* FindState(const uint32_t id)
	{
		auto it = m_stateMap.find(id);
		if (it != m_stateMap.end()) {
			return it->second.get();
		}
		return nullptr;
	}
};
