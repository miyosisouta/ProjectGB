#pragma once

#include "MissionCondition.h"
#include "MissionEvent.h"
#include "MissionType.h"
/**
 * Mission.h
 * 1件のミッションを表すクラス。
 * ミッション名・説明文とMissionConditionを所有し、
 * 条件の結果をもとにミッションの状態遷移(Active / Cleared / Failed)を管理する。
 * UIへ渡すMissionUIDataの生成もこのクラスが担う。
 */

class Mission
{
private:
    MissionID id_ = MissionID::enNone; //!< ミッションのID
    std::wstring name_; //!< ミッション名
    std::unique_ptr<MissionConditionBase> condition_; //!< 達成条件
    MissionState state_ = MissionState::enActive; //!< ミッションの現在の状態
    MissionUpdateType updateType_ = MissionUpdateType::enOneShot; //!< 更新タイプ
    int uiSlot_ = 0; //!< UIスロット番号

    bool clearedThisFrame_ = false;      //!< クリアした瞬間のフレームだけtrue
    bool failedThisFrame_ = false;       //!< 失敗した瞬間のフレームだけtrue
    bool countUpdatedThisFrame_ = false; //!< カウントが増えた瞬間のフレームだけtrue
    bool clearAnimationPlayed_ = false;  //!< クリアアニメーション再生済みフラグ


private:
    /** 条件の結果をもとにミッションの状態を更新する */
    void UpdateState();

    /** 1フレーム限りのフラグをリセットする */
    void ClearFrameFlags();


public:
	/** コンストラクタ */
	Mission(MissionID id,
        std::wstring name,
		std::unique_ptr<MissionConditionBase> condition,
		MissionUpdateType updateType,
		int uiSlot);

public:
    /** 更新処理 */
    void Update();

    /** イベント通知 */
    void OnEvent(const MissionEventData& ev);


public:
    /** ミッションのIDを取得 */
    inline MissionID GetID() const { return id_; }
    /** ミッションの更新タイプを取得 */
    inline MissionUpdateType GetUpdateType() const { return updateType_; }
    /** ミッションの現在の状態を取得する */
    inline MissionState GetState()const { return state_; }

    /** ミッションが進行中か */
    inline bool IsActive()const { return state_ == MissionState::enActive; }

    /** ミッションがクリア済みか */
    inline bool IsCleared()const { return state_ == MissionState::enCleared; }

    /** ミッションが失敗済みか */
    inline bool IsFailed()const { return state_ == MissionState::enFailed; }

    /** クリアした瞬間のフレームだけtrueを返す */
    inline bool IsClearedThisFrame()const { return clearedThisFrame_; }

    /** 失敗した瞬間のフレームだけtrueを返す */
    inline bool IsFailedThisFrame() const { return failedThisFrame_; }

    /** カウントが増えた瞬間のフレームだけtrueを返す */
    inline bool IsCountUpdatedThisFrame()const { return countUpdatedThisFrame_; }

    /** このミッションを表示するUIスロット番号を取得（1=mission_1, 2=mission_2, 3=mission_3） */
    inline int GetUISlot() const { return uiSlot_; }

    /** クリアアニメーションを再生済みか */
    inline bool IsClearAnimationPlayed() const { return clearAnimationPlayed_; }

    /** クリアアニメーション再生済みフラグを立てる */
    inline void SetClearAnimationPlayed() { clearAnimationPlayed_ = true; }


    /* ======================================== */
    /* conditionから情報を取得*/
    /* ======================================== */

    /** 現在のカウントを取得する */
    inline int GetCurrentCount()const { return condition_->GetCurrentCount(); }

    /** 目標のカウントを取得する */
    inline int GetRequiredCount()const { return condition_->GetRequiredCount(); }

    /** 達成進捗率を取得する */
    inline float GetProgress()const { return condition_->GetProgress(); }

};