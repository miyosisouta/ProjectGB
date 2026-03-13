/**
 * InGameScene.cpp
 *
 * インゲームシーンの遷移
 */

#include "stdafx.h"
#include "InGameScene.h"
#include "src/Scene/OutGameScene.h"


InGameScene::InGameScene()
{

}


InGameScene::~InGameScene()
{

}


bool InGameScene::Start() 
{
	return true;
}


void InGameScene::Update()
{

}


bool InGameScene::RequestScene(uint32_t& id)
{
	//Aボタンが押されたら次のシーンへ（仮）
	if (g_pad[0]->IsTrigger(enButtonA)) {
		id = OutGameScene::ID();
		return true;
	}

	return false;
}