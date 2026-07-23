#include "stdafx.h"
#include "Mission.h"

Mission::Mission(MissionID id, std::wstring name, std::unique_ptr<MissionConditionBase> condition, MissionUpdateType updateType, int uiSlot)
	// ID・名前・達成条件（所有権を受け取る）・更新タイプ・UIスロット番号をそのまま保持する
	:id_(id), name_(name), condition_(std::move(condition)), updateType_(updateType), uiSlot_(uiSlot)
{
}

void Mission::Update()
{
    // 前フレームで立てた1フレーム限りのフラグをリセット
    ClearFrameFlags();

    // 進行中（enActive）でなければ、クリア/失敗が確定済みなのでこれ以上判定しない
    if (!IsActive()) return;

    // 経過時間などで進行する条件を1フレーム分進める
    condition_->Update();

    // 進めた結果をもとにミッションの状態(Active/Cleared/Failed)を更新する
    UpdateState();
}

void Mission::OnEvent(const MissionEventData& ev)
{
    // 前フレームで立てた1フレーム限りのフラグをリセット
    ClearFrameFlags();

    // 進行中（enActive）でなければイベントを処理しても意味がないので無視する
    if (!IsActive()) return;

    // イベントを条件クラスへ転送する
    condition_->OnEvent(ev);

    // 条件クラス側でカウントが増えたかどうかを引き継ぎ、UIへの通知用フラグとして保持する
    countUpdatedThisFrame_ = condition_->IsCountUpdatedThisFrame();
    // 条件クラス側のフラグは1フレームで消費したのでクリアしておく
    condition_->ClearCountUpdatedFlag();

    // イベント処理の結果をもとにミッションの状態(Active/Cleared/Failed)を更新する
    UpdateState();
}

void Mission::UpdateState() {
    // 条件がクリア済みなら、ミッションの状態をクリアに切り替える
    if (condition_->IsCleared()) {
        state_ = MissionState::enCleared;
        clearedThisFrame_ = true;  // クリアした瞬間だけtrue（UI側の演出トリガーに使う）
    }
    // クリアしておらず、失敗条件を満たしているなら、ミッションの状態を失敗に切り替える
    else if (condition_->IsFailed()) {
        state_ = MissionState::enFailed;
        failedThisFrame_ = true; // 失敗した瞬間だけtrue
    }
}

void Mission::ClearFrameFlags() {
    // クリア/失敗/カウント更新の1フレーム限りのフラグをすべて下ろす
    clearedThisFrame_ = false;
    failedThisFrame_ = false;
    countUpdatedThisFrame_ = false;
}