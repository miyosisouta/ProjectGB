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
#include "src/UI/CameraOptionMenu.h"


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
	layout_->Initialize<CameraOptionMenu>("Assets/ui/Layout/CameraOptionMenu.json");

	return true;
}


void DebugScene::Update()
{
	//taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	CameraOptionMenu* menu = static_cast<CameraOptionMenu*>(layout_->GetMenu());
	
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