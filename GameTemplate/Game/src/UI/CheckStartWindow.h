/**
 * CheckStartWindow.h
 * 同じボタンが設定された場合に出てくる警告ウィンドウ
 */
#pragma once
#include "Menu.h"


class Layout;
class TaskSchedulerSystem;


class CheckStartWindow : public MenuBase
{
private:
	Layout* layout_;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;

	static bool isCheckWindowClose_;


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

private:
	void BackButtonAnim();
	void StartButtonAnim();
	void ResetIcon();

public:
	static void SetCheckWindowClose(const bool isClose) { isCheckWindowClose_ = isClose; }

	/** もどるを選んでいるか */
	bool IsSelectBackButton() const;
	/** はじめるを選んでいるか */
	bool IsSelectStartButton() const;
};