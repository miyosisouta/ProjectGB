/**
 * InGameScene.cpp
 *
 * インゲームシーンの遷移
 */

#include "stdafx.h"
#include "InGameScene.h"

#include "src/Core/BattleManager.h"
#include "src/Scene/OutGameScene.h"
#include "src/Sound/SoundManager.h"

#include "src/UI/Layout.h"
#include "src/UI/InGameMenu.h"


InGameScene::InGameScene()
{
	BattleManager::CreateInstance();		//BattleManagerのインスタンスを作成
	
}


InGameScene::~InGameScene()
{
	BattleManager::Get().DestroyInstance();		//BattleManagerのインスタンスを破棄	
	delete layout_;
}


bool InGameScene::Start() 
{
	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<InGameMenu>("Assets/ui/layout/InGameMenu.json");

	// インゲームBGM再生
	SoundManager::Get().PlayBGM(enSoundKind_InGame);

	return true;
}


void InGameScene::Update()
{
	BattleManager::Get().Update();		//BattleManagerの更新
	
	layout_->Update(); // 更新の呼び出し
}


void InGameScene::Render(RenderContext& rc)
{
	layout_->Render(rc);
}


bool InGameScene::RequestScene(uint32_t& id)
{
	////Aボタンが押されたら次のシーンへ（仮）
	//if (g_pad[0]->IsTrigger(enButtonA)) {
	//	id = OutGameScene::ID();
	// 
	// // タイトルBGM停止
	// SoudManager::Get().StopBGM();
	// 
	//	return true;
	//}

	return false;
}