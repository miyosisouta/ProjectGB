/**
 * GameTimer.h
 * ゲームの制限時間を管理するクラス（カウントアップ方式）
 */
#pragma once

namespace
{
    constexpr uint8_t TIMER_MINUTE_PER_SECOND = 60; //!< 分と秒を分けるための変数
}

class GameTimer {
private:
    float limitTime_ = 0.0f;         //!< 制限時間
    float elapsedTime_ = 0.0f;       //!< 経過時間
    float remainWarningTime_ = 0.0f; //!< 警告開始時間
    bool isRunning_ = false;        //!< 時間を計算するかフラグ
    bool isTimeUp_ = false;         //!< タイムアップかフラグ
    bool isWarningFrame_ = false;   //!< 残り時間が設定した時間を切ったフレームのフラグ
    bool hasWarned_ = false;        //!< 警告を既に発火済みかフラグ
    bool isPaused_ = false;         //!< ポーズ画面を開いているかのフラグ

public:
    /** 初期設定 */
    void Init();
    /** 更新 */
    void Update();
    /** 初期化 */
    void Reset();

    /** 時間を計算させる */
    inline void Start() { isRunning_ = true; }
    /** 時間の計算を止める */
    inline void Stop() { isRunning_ = false; }
    /** ポーズ画面を開いている */
    inline void Pause() { isPaused_ = true; }
    /** ポーズ画面を閉じた */
    inline void Resume() { isPaused_ = false; }


    /******* UI向け取得関数 *******/

    /** 制限時間の設定 : Init()を呼んでから設定してください */
    inline void SetLimitTime(const float time) { limitTime_ = time; }
    /** 警告時間の設定 : Init()を呼んでから設定してください */
    inline void SetWarningTime(const float time) { remainWarningTime_ = time; }
    /** 経過時間 */
    inline float GetElapsedTime()    const { return elapsedTime_; }
    /** 残り時間 */
    inline float GetRemainTime()     const { return limitTime_ - elapsedTime_; }
    /** 残り時間 : 秒 */
    inline int GetRemainSeconds()  const { return (int)GetRemainTime() % TIMER_MINUTE_PER_SECOND; }
    /** 残り時間 : 分 */
    inline int GetRemainMinutes()  const { return (int)GetRemainTime() / TIMER_MINUTE_PER_SECOND; }
    /** 残り時間の割合 */
    inline float GetRate()           const { return GetRemainTime() / limitTime_; }


    /****** 状態取得 ******/

    /** タイムアップかどうかを取得 */
    inline bool IsTimeUp()  const { return isTimeUp_; }
    /** 時間を進めているかを取得 */
    inline bool IsRunning() const { return isRunning_; }
    /** ポーズ画面を開いているかを取得 */
    inline bool IsPaused()  const { return isPaused_; }
    /** 警告時間を切っているかを取得 */
    inline bool IsWarningFrame() const { return isWarningFrame_; }
};