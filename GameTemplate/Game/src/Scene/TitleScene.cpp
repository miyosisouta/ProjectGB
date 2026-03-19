/**
 * TitleScene.h
 *
 * タイトルシーンの遷移
 */

#include "stdafx.h"
#include "TitleScene.h"

#include "src/Scene/InGameScene.h"
#include "src/Scene/TitleScreen.h"


TitleScene::TitleScene()
{
}


TitleScene::~TitleScene()
{
	DeleteGO(titleScreen_);
}


bool TitleScene::Start()
{
	titleScreen_ = NewGO<TitleScreen>(0, "titleScreen");


	return true;
}


void TitleScene::Update()
{
}


bool TitleScene::RequestScene(uint32_t& id)
{
	//Aボタンが押されたら次のシーンへ（仮）
	if (g_pad[0]->IsTrigger(enButtonA)) {
		id = InGameScene::ID();
		return true;
	}

	return false;
}