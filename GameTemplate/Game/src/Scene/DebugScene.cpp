/**
 * DebugScene.cpp
 *
 * デバックの描画
 */

#include "stdafx.h"
#include "DebugScene.h"
#include "TitleScene.h"

#include "src/sound/SoundManager.h"
#include "src/Util/TaskSchedulerSystem.h"

#include "src/UI/Layout.h"
#include "src/UI/KeyConfigOptionMenu.h"


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
	layout_->Initialize<KeyConfigOptionMenu>("Assets/ui/Layout/KeyConfigOptionMenu.json");

	return true;
}


void DebugScene::Update()
{
	KeyConfigOptionMenu* menu = static_cast<KeyConfigOptionMenu*>(layout_->GetMenu());
	

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