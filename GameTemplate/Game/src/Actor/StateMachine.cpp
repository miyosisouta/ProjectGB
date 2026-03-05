#include "stdafx.h"
#include "StateMachine.h"

void IStateMachine::Update()
{
	UpdateState();
}


void IStateMachine::UpdateState()
{
	if (m_nextStateId != INVALID_STATE_ID) {
		if (m_currentState) {
			m_currentState->Exit();
		}
		auto* nextState = FindState(m_nextStateId);
		nextState->Enter();
		m_currentState = nextState;
		m_currentStateId = m_nextStateId;
		m_nextStateId = INVALID_STATE_ID;
	}
	m_currentState->Update();
}