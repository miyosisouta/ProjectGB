/**
 * OutGameScene.cpp
 *
 * アウトゲームシーンの遷移
 */

#include "stdafx.h"
#include "OutGameScene.h"
#include "src/Scene/InGameScene.h"


OutGameScene::OutGameScene()
{

}


OutGameScene::~OutGameScene()
{

}


bool OutGameScene::Start()
{
	return true;
}


void OutGameScene::Update()
{

}


bool OutGameScene::RequestScene(uint32_t& id)
{
	//Aボタンが押されたら次のシーンへ（仮）
	if (g_pad[0]->IsTrigger(enButtonA)) {
		id = InGameScene::ID();
		return true;
	}

	return false;
}