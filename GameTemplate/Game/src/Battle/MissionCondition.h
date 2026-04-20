#pragma once

/*
 * MissionCondition.h
 * ミッションの達成条件を表すクラス
 * 毎フレームの状態監視とイベント検知のインターフェースを提供する。
 * カウント更新時のUIアニメーション通知フラグの管理もこのクラスが担う。
 * 具体的な条件ロジックはMissionConditionsに派生クラスとして実装
 */

#include "MissionEvent.h"
#include <algorithm>


/*=============================================*/
/* ミッションの基底クラス */
/*=============================================*/
class MissionConditionBase {
public:
    virtual ~MissionConditionBase() = default;

    /* 更新処理 */
    virtual void Update() {}

    /* イベント */
    virtual void OnEvent(const MissionEventData& ev) {}

    /* クリアしたか */
    inline virtual bool  IsCleared() const = 0;
    /* 失敗したか */
    inline virtual bool  IsFailed() const { return false; }
    /* 現在の進捗を取得 */
    inline virtual float GetProgress() const { return IsCleared() ? 1.0f : 0.0f; }
    /* 現在の値を取得 */
    inline virtual uint8_t GetCurrentCount() const { return 0; }
    /* クリアに必要な値を取得 */
    inline virtual uint8_t GetRequiredCount() const { return 0; }

    // カウントが更新されたフレームだけtrue。Mission側が読んでUIDataに反映する
    inline bool IsCountUpdatedThisFrame() const { return countUpdatedThisFrame_; }
    /* 値を更新するかのフラグ */
    inline void ClearCountUpdatedFlag() { countUpdatedThisFrame_ = false; }

protected:
    // 派生クラスがカウントを増やしたときに呼ぶ
    inline void NotifyCountUpdated() { countUpdatedThisFrame_ = true; }

private:
    bool countUpdatedThisFrame_ = false; //!< カウントが増えたときに1フレームだけtrue
};



/*=============================================*/
/* 特殊スキルの使用回数の条件 */
/*=============================================*/

class ConditionUseAbilityCount : public MissionConditionBase {
public:
    /* targetType = enNone なら全アビリティ対象 */
    ConditionUseAbilityCount(int requiredCount,
        AbilityType targetType = AbilityType::enNone);

    /* イベント */
    void  OnEvent(const MissionEventData& ev) override;
    /* クリアしたか */
    inline bool  IsCleared()const override { return isCleared_; }
    /* 進捗を取得 */
    inline float GetProgress()const override { 
        if (required_ == 0) return 0.0f;
        return min(1.0f, (float)count_ / (float)required_);
    }
    /* 現在の値を取得 */
    inline uint8_t GetCurrentCount()const override { return count_; }
    /* クリアに必要な値を取得 */
    inline uint8_t GetRequiredCount()const override {return required_;}

private:
    uint8_t required_ = 0; //!< クリアの値
    AbilityType targetType_ = AbilityType::enNone; //!< 条件のタイプ
    uint8_t count_ = 0; //!< 現在の値
    bool isCleared_ = false; // クリアしたか
};



/*=============================================*/
/* 回避スキルの使用回数の条件 */
/*=============================================*/

class ConditionUseUtilityCount : public MissionConditionBase {
public:
    /* targetType = enNone なら全アビリティ対象 */
    ConditionUseUtilityCount(int requiredCount,
        UtilityType targetType = UtilityType::enNone);

    /* イベント */
    void  OnEvent(const MissionEventData& ev) override;
    /* クリアしたか */
    inline bool  IsCleared()const override { return isCleared_; }
    /* 進捗を取得 */
    inline float GetProgress()const override { 
        if (required_ == 0) return 0.0f;
        return min(1.0f, (float)count_ / (float)required_);
    }
    /* 現在の値を取得 */
    inline uint8_t GetCurrentCount()const override { return count_; }
    /* クリアに必要な値を取得 */
    inline uint8_t GetRequiredCount()const override { return required_; }

private:
    uint8_t required_ = 0; //!< クリアの値
    UtilityType targetType_ = UtilityType::enNone; //!< 条件のタイプ
    uint8_t count_ = 0; //!< 現在の値
    bool isCleared_ = false; // クリアしたか
};



/*=============================================*/
/* 時間内撃破 */
/*=============================================*/
class ConditionKillUnderTime : public MissionConditionBase {
public:
    /* explicit : 型変換を防ぐ */
    explicit ConditionKillUnderTime(float timeLimitSec);

    /* 更新 */
    void  Update()override;
    /* イベント */
    void  OnEvent(const MissionEventData& ev) override;
    /* クリアしたか */
    inline bool  IsCleared()const override { if (!isBossDefeated_) return false; return elapsed_ <= timeLimit_; }
    /* 失敗したか */
    inline bool IsFailed() const override{ return !isBossDefeated_ && elapsed_ > timeLimit_; }
    /* 進捗を取得 */
    inline float GetProgress()const override { 
        if (timeLimit_ == 0.0f) return 0.0f;
        return max(0.0f, 1.0f - (elapsed_ / timeLimit_));
    }
    /* 現在の値を取得 */
    inline uint8_t GetCurrentCount()const override { return elapsed_; }
    /* クリアに必要な値を取得 */
    inline uint8_t GetRequiredCount()const override { return timeLimit_; }

private:
    float timeLimit_ = 0.0f; //!< 制限時間
    float elapsed_ = 0.0f; //!< 経過時間
    bool  isBossDefeated_ = false; //!< ボスを倒したか
};