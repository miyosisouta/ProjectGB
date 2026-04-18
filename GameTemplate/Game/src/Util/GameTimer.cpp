#include "stdafx.h"
#include "GameTimer.h"

namespace
{
	constexpr float WARNING_TIME = 60.0f; // 警告時間
}


void GameTimer::Init(float limitTime)
{
	limitTime_ = limitTime;				// 制限時間のMAXを保存
	remainTime_ = limitTime;			// 残り時間を設定
	remainWarningTime_ = WARNING_TIME;	// 警告時間を設定
	isRunning_ = true;					// 時間を経過させるか
}

void GameTimer::Update()
{
	// 動かさないもしくはポーズ画面中ならそもそも処理しない
	if (!isRunning_ || isPaused_) { return; }

	// 時間を減らす
	remainTime_ -= g_gameTime->GetFrameDeltaTime();

	// 警告時間か否か
	if (!isWarning_ && remainTime_ <= remainWarningTime_){ isWarning_ = true; }

	// 制限時間が来たか
	if (remainTime_ <= 0.0f) { 
		remainTime_ = 0.0f; // 制限時間を0に
		isTimeUp_ = true;	// タイムアップフラグをtrueに
		isRunning_ = true;	// これ以上時間の計算を行わない
	}
}

void GameTimer::Reset()
{
	limitTime_ = 0.0f;         //!< 制限時間
	remainTime_ = 0.0f;        //!< 残り時間
	remainWarningTime_ = 0.0f; //!< 警告時間
	isRunning_ = false; //!< 動作中フラグ
	isTimeUp_ = false;  //!< タイムアップフラグ
	isWarning_ = false; //!< 残り時間が一定時間を切っているか
	isPaused_ = false;  //!< ポーズ中フラグ
}
