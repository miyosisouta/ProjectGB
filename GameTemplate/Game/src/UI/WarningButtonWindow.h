/**
 * WarningButtonWindow.h
 * 同じボタンが設定された場合に出てくる警告ウィンドウ
 */
#pragma once
#include <memory>
#include "Menu.h"


class Layout;
class RenderContext;
class TaskSchedulerSystem;


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