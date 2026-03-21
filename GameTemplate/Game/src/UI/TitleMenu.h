/**
 * TitleMenu.h
 * タイトルメニュー
 */
#pragma once
#include "Menu.h"


class TitleMenu : public MenuBase
{
private:
	TaskSchedulerSystem* taskScheduler = nullptr;
	std::unique_ptr<IntSelector> selector_ = nullptr;
	/** Aボタンを押したかがされているか */
	bool isAbuttonEnabled = false;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

	/** 「はじめる」を選んでいるか */
	bool IsSelectStat() const;
	/** 「サウンド」を選んでいるか */
	bool IsSelectSound() const;
	/** 「おわり」を選んでいるか */
	bool IsSelectExit() const;
	/** Aボタンを押したかがされているか */
	bool IsAbuttonEnabled() const
	{
		return isAbuttonEnabled;
	}
};