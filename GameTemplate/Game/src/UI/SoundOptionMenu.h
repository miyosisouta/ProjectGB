/**
 * SoundOptionMenu.h
 * サウンドオプションメニュー
 */
#pragma once
#include "Menu.h"


class SoundOptionMenu : public MenuBase
{
public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};