/**
 * TitleMenu.h
 * タイトルメニュー
 */
#pragma once
#include "Menu.h"


class TitleMenu : public MenuBase
{
private:
	

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};