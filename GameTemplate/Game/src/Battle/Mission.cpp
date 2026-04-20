#include "stdafx.h"
#include "Mission.h"

Mission::Mission(std::wstring name, std::unique_ptr<MissionConditionBase> condition)
	:name_(name), condition_(std::move(condition))
{
}

void Mission::Update() 
{
    // 毎フレームフラグをリセット
    ClearFrameFlags();

    // 進行してないなら返す
    if (!IsActive()) return;
    // ミッションの状態を更新
    condition_->Update();

    // 結果をもとにステートを更新
    UpdateState();
}

void Mission::OnEvent(const MissionEventData& ev) 
{
    // 毎フレームフラグをリセット
    ClearFrameFlags();

    // 進行してないなら返す
    if (!IsActive()) return;

    // ミッションの状態イベントを呼ぶ
    condition_->OnEvent(ev);

    // カウント更新フラグをConditionから引き継ぐ
    countUpdatedThisFrame_ = condition_->IsCountUpdatedThisFrame();
    condition_->ClearCountUpdatedFlag();

    // ステートを更新
    UpdateState();
}

void Mission::UpdateState() {
    // クリアしているなら
    if (condition_->IsCleared()) {
        state_ = MissionState::enCleared;
        clearedThisFrame_ = true;  // クリアした瞬間だけtrue
    }
    // 失敗をしているなら
    else if (condition_->IsFailed()) {
        state_ = MissionState::enFailed;
        failedThisFrame_ = true; // 失敗した瞬間だけtrue
    }
}

void Mission::ClearFrameFlags() {
    clearedThisFrame_ = false;
    failedThisFrame_ = false; 
    countUpdatedThisFrame_ = false;
}