/**
 * InGameScene.cpp
 *
 * インゲームシーンの遷移
 */

#include "stdafx.h"
#include "InGameScene.h"

#include "src/Core/BattleManager.h"
#include "src/UI/InGameUIManager.h"
#include "src/Scene/OutGameScene.h"


InGameScene::InGameScene()
{
	BattleManager::CreateInstance();		//BattleManagerのインスタンスを作成
	InGameUIManager::CreateInstance();		//InGameUIManagerのインスタンスを作成
}


InGameScene::~InGameScene()
{
	BattleManager::Get().DestroyInstance();		//BattleManagerのインスタンスを破棄	
	InGameUIManager::Get().DestroyInstance();	//InGameUIManagerのインスタンスを破棄
}


bool InGameScene::Start() 
{
	return true;
}


void InGameScene::Update()
{
	BattleManager::Get().Update();		//BattleManagerの更新
	InGameUIManager::Get().Update();	//InGameUIManagerの更新
}


bool InGameScene::RequestScene(uint32_t& id)
{
	////Aボタンが押されたら次のシーンへ（仮）
	//if (g_pad[0]->IsTrigger(enButtonA)) {
	//	id = OutGameScene::ID();
	//	return true;
	//}

	return false;
}