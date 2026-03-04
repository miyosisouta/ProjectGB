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
	//todo for test
	if(g_pad[0]->IsTrigger(enButtonA))	
}