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
#include "src/UI/TimerMenu.h"


namespace
{
	float ONE_LAP = 360.0f;
}


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
	layout_->Initialize<TimerMenu>("Assets/ui/Layout/TimerMenu.json");

	return true;
}


void DebugScene::Update()
{
	TimerMenu* menu = static_cast<TimerMenu*>(layout_->GetMenu());
	//menu->SetCurrentTimer();
	//menu->SetMaxTimer(180.0f);

	


	layout_->Update();

	//時間
	layout_->GetMenu();
}


void DebugScene::Render(RenderContext& rc)
{
	layout_->Render(rc);
}


bool DebugScene::RequestScene(uint32_t& id)
{
	return false;
}