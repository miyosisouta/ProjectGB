/**
 * TimerMenu.h
 * 制限時間
 */
#pragma once
#include "Menu.h"


class TimerMenu : public MenuBase
{
private:
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_ = nullptr;


private:
	/** 回転の割合(進捗率) */
	float progressRate_ = 0.0f;
	/** 分単位 */
	int minutes_ = 0;
	/** 秒単位 */
	int seconds_ = 0;
	/** 点滅開始 */
	bool flashingStart_ = false;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


public:
	/** 分のセッター 外から持ってくることができる */
	void SetMinutes(const int minutes) { minutes_ = minutes; }
	/** 秒のセッター */
	void SetSeconds(const int seconds) { seconds_ = seconds; }
	/** 回転の割合 */
	void SetRate(const float progressRate) { progressRate_ = progressRate; }
	/** 点滅開始 */
	void SetRequestFlashing(const bool flashingStart) { flashingStart_ = flashingStart; }
};