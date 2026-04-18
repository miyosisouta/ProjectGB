/**
 * GameTimer.cpp
 * ゲームの制限時間を管理するクラス
 */
#pragma once

namespace
{
    constexpr uint8_t TIMER_MINUTE_PER_SECOND = 60;
    constexpr float LIMIT_TIME = 600.0f; // 制限時間
}

class GameTimer {
private:
    float limitTime_ = 0.0f;         //!< 制限時間
    float remainTime_ = 0.0f;        //!< 残り時間
    float remainWarningTime_ = 0.0f; //!< 警告時間
    bool isRunning_ = false; //!< 動作中フラグ
    bool isTimeUp_ = false;  //!< タイムアップフラグ
    bool isWarning_ = false; //!< 残り時間が一定時間を切っているか
    bool isPaused_ = false;  //!< ポーズ中フラグ

public:
    /** 初期設定 : 制限時間の初期設定は10分 */
    void Init(float limitTime = LIMIT_TIME);
    /** 更新 */
    void Update();
    /** 初期化 */
    void Reset();

    /** 計測開始 */
    void Start() { isRunning_ = true; }               
    /** 停止 */
    void Stop() { isRunning_ = false; }
    /** 一時停止 */
    void Pause() { isPaused_ = true; }
    /** 再開 */
    void Resume() { isPaused_ = false; }


    /******* UI向け取得関数 *******/

    /** 残り時間(float) */
    float GetRemainTime() const { return remainTime_; }                                     
    /** 残り時間(秒) */
    float GetRemainSeconds() const { return (int)remainTime_ % TIMER_MINUTE_PER_SECOND; }   
    /** 残り時間(分) */
    int   GetRemainMinutes() const { return remainTime_ / TIMER_MINUTE_PER_SECOND; }        
    /** 残り時間の割合 */
    float GetRate() const { return remainTime_ / limitTime_; };                             
    

    /****** 状態取得 ******/

    /** タイムアップか */
    bool  IsTimeUp() const { return isTimeUp_; }
    /** 計算中か */
    bool  IsRunning() const { return isRunning_; }
    /** ポーズ中か */
    bool  IsPaused() const { return isPaused_; }
    /** 警告状態か*/
    bool  IsWarning() const { return isWarning_; }
    /** 警告時間を取得 */
    float GetWarningRate() const { return remainWarningTime_; }
};

