#pragma once

#include "MissionCondition.h"
#include "MissionEvent.h"
#include "MissionType.h"
/*
 * Mission.h 
 * 1件のミッションを表すクラス。
 * ミッション名・説明文とMissionConditionを所有し、
 * 条件の結果をもとにミッションの状態遷移(Active / Cleared / Failed)を管理する。
 * UIへ渡すMissionUIDataの生成もこのクラスが担う。
 */

class Mission
{
public:
	Mission(std::wstring name,
		std::unique_ptr<MissionConditionBase> condition);

public:
    /** 更新処理 */
    void Update();

    /** イベント通知 */
    void OnEvent(const MissionEventData& ev);


public:
    /** ミッションの現在の状態を取得する */
    MissionState GetState()const { return state_; }

    /** ミッションが進行中か */
    bool IsActive()const { return state_ == MissionState::enActive; }

    /** ミッションがクリア済みか */
    bool IsCleared()const { return state_ == MissionState::enCleared; }

    /** ミッションが失敗済みか */
    bool IsFailed()const { return state_ == MissionState::enFailed; }

    /** クリアした瞬間のフレームだけtrueを返す */
    bool IsClearedThisFrame()const { return clearedThisFrame_; }

    /** 失敗した瞬間のフレームだけtrueを返す */
    bool IsFailedThisFrame() const { return failedThisFrame_; }

    /** カウントが増えた瞬間のフレームだけtrueを返す */
    bool IsCountUpdatedThisFrame()const { return countUpdatedThisFrame_; }


    /* ======================================== */
    /* conditionから情報を取得*/
    /* ======================================== */

    /** 現在のカウントを取得する */
    int   GetCurrentCount()const { return condition_->GetCurrentCount(); }

    /** 目標のカウントを取得する */
    int   GetRequiredCount()const { return condition_->GetRequiredCount(); }

    /** 達成進捗率を取得する */
    float GetProgress()const { return condition_->GetProgress(); }

private:
    /** 条件の結果をもとにミッションの状態を更新する */
    void UpdateState();

    /** 1フレーム限りのフラグをリセットする */
    void ClearFrameFlags();

private:
    std::wstring name_; //!< ミッション名
    std::unique_ptr<MissionConditionBase> condition_; //!< 達成条件
    MissionState state_ = MissionState::enActive; //!< ミッションの現在の状態

    bool clearedThisFrame_ = false; //!< クリアした瞬間のフレームだけtrue
    bool failedThisFrame_ = false; //!< 失敗した瞬間のフレームだけtrue
    bool countUpdatedThisFrame_ = false; //!< カウントが増えた瞬間のフレームだけtrue
};