/**
 * WarningButtonWindow.h
 * 同じボタンが設定された場合に出てくる警告ウィンドウ
 */
#pragma once
#include "WarningButtonWindow.h"


class Layout;


class WarningButtonWindow : public MenuBase
{
private:
	Layout* layout_;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};