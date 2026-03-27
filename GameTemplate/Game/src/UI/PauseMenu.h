/**
 * PauseMenu.h
 * ポーズメニュー
 */
#pragma once
#include "Menu.h"


class PauseMenu : public MenuBase
{
private:
	bool isReturnTitle_ = false;
	//bool isCloseMenu_ = false;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


public:
	bool IsReturnTitle() const { return isReturnTitle_; }
	//bool IsCloseMenu() const { return isCloseMenu_; }
};