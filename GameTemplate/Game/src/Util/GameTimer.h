/**
 * GameTimer.h
 * ゲームの制限時間を管理するクラス（カウントアップ方式）
 */
#pragma once

namespace
{
    constexpr uint8_t TIMER_MINUTE_PER_SECOND = 60; //!< 分と秒を分けるための変数
    constexpr float LIMIT_TIME = 600.0f; //!< 制限時間（10分）
}

class GameTimer {
private:
    float limitTime_ = 0.0f;         //!< 制限時間
    float elapsedTime_ = 0.0f;       //!< 経過時間（0からカウントアップ）
    float remainWarningTime_ = 0.0f; //!< 警告開始時間（経過時間がこれを超えたら警告）
    bool isRunning_ = false; //!< 時間を計算するかフラグ
    bool isTimeUp_ = false;  //!< タイムアップかフラグ
    bool isWarning_ = false; //!< 残り時間が60秒切ったかフラグ
    bool isPaused_ = false;  //!< ポーズ画面を開いているかのフラグ

public:
    /** 初期設定 */
    void Init(float limitTime = LIMIT_TIME);
    /** 更新 */
    void Update();
    /** 初期化 */
    void Reset();

    /** 時間を計算させる */
    void Start() { isRunning_ = true; }
    /** 時間の計算を止める */
    void Stop() { isRunning_ = false; }
    /** ポーズ画面を開いている */
    void Pause() { isPaused_ = true; }
    /** ポーズ画面を閉じた */
    void Resume() { isPaused_ = false; }


    /******* UI向け取得関数 *******/

    /** 経過時間 */
    float GetElapsedTime()    const { return elapsedTime_; }
    /** 残り時間 */
    float GetRemainTime()     const { return limitTime_ - elapsedTime_; }
    /** 残り時間 */
    int   GetRemainSeconds()  const { return (int)GetRemainTime() % TIMER_MINUTE_PER_SECOND; }
    /** 残り時間 */
    int   GetRemainMinutes()  const { return (int)GetRemainTime() / TIMER_MINUTE_PER_SECOND; }
    /** 残り時間の割合 */
    float GetRate()           const { return GetRemainTime() / limitTime_; }


    /****** 状態取得 ******/

    /** タイムアップかどうかを取得 */
    bool IsTimeUp()  const { return isTimeUp_; }
    /** 時間を進めているかを取得 */
    bool IsRunning() const { return isRunning_; }
    /** ポーズ画面を開いているかを取得 */
    bool IsPaused()  const { return isPaused_; }
    /** 60秒を切っているかを取得 */
    bool IsWarning() const { return isWarning_; }
};