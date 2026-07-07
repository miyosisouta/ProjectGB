/**
 * DebugScene.cpp
 *
 * デバックの描画
 */

#include "stdafx.h"
#include "DebugScene.h"
#include "TitleScene.h"
#include "src/Scene/TitleBackground.h"

#include "src/sound/SoundManager.h"
#include "src/Util/TaskSchedulerSystem.h"


#include "src/UI/Layout.h"
#include "src/UI/TitleMenu.h"


DebugScene::DebugScene()
{
	//titleBackground_ = NewGO<TitleBackground>(0, "SoundOptionMenu");
}


DebugScene::~DebugScene()
{
	DeleteGO(titleBackground_);
	// UI
	//delete layout_;
}


bool DebugScene::Start()
{
	// UI
	layout_ = new Layout;
	layout_->Initialize<TitleMenu>("Assets/ui/Layout/TitleMenu.json");


	// タイトル


	return true;
}


void DebugScene::Update()
{
	// UI
	//taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	TitleMenu* menu = static_cast<TitleMenu*>(layout_->GetMenu());
	
	layout_->Update();
}


void DebugScene::Render(RenderContext& rc)
{
	// UI
	layout_->Render(rc);
}


bool DebugScene::RequestScene(uint32_t& id)
{
	return false;
}