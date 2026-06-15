/**
 * CameraOptionMenu.h
 * カメラオプションメニュー
 */
#pragma once
#include "Menu.h"


class Layout;


class CameraOptionMenu : public MenuBase
{
public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;
};