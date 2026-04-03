/**
 * BootScene.cpp
 *
 * タイトル画面の描画
 */

#include "stdafx.h"
#include "BootScene.h"
#include "TitleScene.h"
#include "src/sound/SoundManager.h"
#include "src/UI/Layout.h"
#include "src/UI/BootMenu.h"


BootScene::BootScene()
{
}


BootScene::~BootScene()
{
	delete layout_;
}


bool BootScene::Start()
{
	layout_ = new Layout;
	layout_->Initialize<BootMenu>("Assets/ui/Layout/BootSceneMenu.json");

	return true;
}


void BootScene::Update()
{
	layout_->Update();
}


void BootScene::Render(RenderContext& rc)
{
	layout_->Render(rc);
}


bool BootScene::RequestScene(uint32_t& id)
{
	auto* menu = static_cast<BootMenu*>(layout_->GetMenu());
	if (menu) {
		if (menu->IsCompleted()) {
			id = TitleScene::ID();
			return true;
		}
	}

	return false;
}