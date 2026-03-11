#include "stdafx.h"
#include "StateMachine.h"

void StateMachine::Update()
{
	// まずグローバル遷移をチェック（死んだり被弾したりしていないか）
	for (const auto& transition : globalTransitions_) { // 最優先すべき遷移ルール
		if (transition.condition()) { // 条件を満たした場合
			nextStateId_ = transition.nextState;
			break;
		}
	}

	// 次に現在のステートの遷移をチェック
	if (nextStateId_ == StateID::None && currentStateId_ != StateID::None) {
		auto it = stateTransitions_.find(currentStateId_); // 状態を見つける
		if (it != stateTransitions_.end()) { // リストの中になかったら
			for (const auto& transition : it->second) {
				if (transition.condition()) { // 条件を満たしたら！
					nextStateId_ = transition.nextState;
					break;
				}
			}
		}
	}

	// ステートの切り替えと更新
	UpdateState();
}


void StateMachine::UpdateState()
{
	// 次のステートのIDが設定されている場合
	if (nextStateId_ != StateID::None) 
	{
		if (currentState_)
		{
			currentState_->Exit(); // 現在のフェーズ終了処理
		}

		auto* nextState = FindState(nextStateId_); // 特定のIDのステートを見つける

		if (nextState) // 特定のIDのステートがあるなら
		{
			nextState->Enter(); // 次のステートのフェーズ開始処理
			currentState_ = nextState; // 次のステートを現在のステートとする
			currentStateId_ = nextStateId_; // IDも同じく設定
		}
	}
	// 次のステートIDを何も入れない
	nextStateId_ = StateID::None; 

	// 現在のステートがある場合
	if (currentState_)
	{
		// 現在のステートの更新
		currentState_->Update();
	}
}
