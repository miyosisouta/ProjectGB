/**
 * InGameMenu.h
 * インゲームメニュー
 */
#pragma once
#include "Menu.h"


class InGameMenu : public MenuBase
{
public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};