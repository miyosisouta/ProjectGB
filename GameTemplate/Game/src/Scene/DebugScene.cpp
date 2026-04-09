/**
 * DebugScene.cpp
 *
 * デバックの描画
 */

#include "stdafx.h"
#include "DebugScene.h"
#include "TitleScene.h"

#include "src/sound/SoundManager.h"
#include "src/UI/Layout.h"
#include "src/UI/BootMenu.h"


DebugScene::DebugScene()
{
}


DebugScene::~DebugScene()
{
	delete layout_;
}


bool DebugScene::Start()
{
	layout_ = new Layout;
	layout_->Initialize<MenuBase>("Assets/ui/Layout/SoundOptionMenu.json");


	return true;
}


void DebugScene::Update()
{
	layout_->Update();
}


void DebugScene::Render(RenderContext& rc)
{
	layout_->Render(rc);
}


bool DebugScene::RequestScene(uint32_t& id)
{
	return false;
}