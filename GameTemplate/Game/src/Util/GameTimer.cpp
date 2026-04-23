#include "stdafx.h"
#include "GameTimer.h"



void GameTimer::Init()
{
    limitTime_ = 0.0f;                             // 制限時間を保存
    elapsedTime_ = 0.0f;                           // 経過時間をリセット
    isRunning_ = true;                             // 時間が進むように設定
}

void GameTimer::Update()
{
    if (!isRunning_ || isPaused_) { return; }

    // 時間を増やす
    elapsedTime_ += g_gameTime->GetFrameDeltaTime();

    // 警告判定：未発火かつ警告時間を超えたフレームだけtrue
    if (!hasWarned_ && elapsedTime_ >= remainWarningTime_) {
        isWarningFrame_ = true;
        hasWarned_ = true; // 以降は発火しない
    }

    // タイムアップ判定
    if (elapsedTime_ >= limitTime_) {
        elapsedTime_ = limitTime_;
        isTimeUp_ = true;
        isRunning_ = false; // これ以上カウントしない
    }
}

void GameTimer::Reset()
{
    limitTime_ = 0.0f;
    elapsedTime_ = 0.0f;
    remainWarningTime_ = 0.0f;
    isRunning_ = false;
    isTimeUp_ = false;
    isWarningFrame_ = false;
    hasWarned_ = false;
    isPaused_ = false;
}