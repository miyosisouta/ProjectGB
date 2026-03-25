/**
 * SoundOptionMenu.h
 * サウンドオプションメニュー
 */
#pragma once
#include "Menu.h"


class SoundOptionMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;


private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;
	UIIcon* nikukyuList[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};