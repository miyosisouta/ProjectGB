/**
 * LoadingMenu.h
 * インゲームメニュー
 */
#pragma once
#include "Menu.h"


class LoadingMenu : public MenuBase
{
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};