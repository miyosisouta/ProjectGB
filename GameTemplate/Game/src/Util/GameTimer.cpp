#include "stdafx.h"
#include "GameTimer.h"

namespace
{
    constexpr float WARNING_TIME = 60.0f; // 警告時間（残り60秒で警告）
}

void GameTimer::Init(float limitTime)
{
    limitTime_ = limitTime;                        // 制限時間を保存
    elapsedTime_ = 0.0f;                           // 経過時間をリセット
    remainWarningTime_ = limitTime - WARNING_TIME; // 警告開始時間（540秒経過で警告）
    isRunning_ = true;
}

void GameTimer::Update()
{
    if (!isRunning_ || isPaused_) { return; }

    // 時間を増やす
    elapsedTime_ += g_gameTime->GetFrameDeltaTime();

    // 警告判定（経過時間が警告時間を超えたら）
    if (!isWarning_ && elapsedTime_ >= remainWarningTime_) { isWarning_ = true; }

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
    isWarning_ = false;
    isPaused_ = false;
}