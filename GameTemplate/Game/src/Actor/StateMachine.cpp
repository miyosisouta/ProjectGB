#include "stdafx.h"
#include "StateMachine.h"

void IStateMachine::Update()
{
	// ステートの更新
	UpdateState();
}


void IStateMachine::UpdateState()
{
	// 次のステートのIDが設定されている場合
	if (m_nextStateId != INVALID_STATE_ID) {
		if (m_currentState) {
			m_currentState->Exit(); // 現在のフェーズ終了処理
		}
		auto* nextState = FindState(m_nextStateId); // 特定のIDのステートを見つける
		nextState->Enter(); // 次のステートのフェーズ開始処理
		m_currentState = nextState; // 次のステートを現在のステートとする
		m_currentStateId = m_nextStateId; // IDも同じく設定
		m_nextStateId = INVALID_STATE_ID; // 次のステートIDを何も入れない
	}
	// 現在のステートの更新
	m_currentState->Update();
}