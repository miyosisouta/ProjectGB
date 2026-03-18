/**
 * InGameUIManager.cpp
 * 
 * インゲームのUIを管理
 */


#include "stdafx.h"
#include "InGameUIManager.h"

#include "src/UI/Layout.h"
#include "src/UI/SoundOptionMenu.h"
#include "src/UI/UIAnimationParameter.h"


namespace
{
	static Layout* layout = nullptr;
}


InGameUIManager* InGameUIManager::myInstance_ = nullptr; //初期化


InGameUIManager::InGameUIManager()
{
	UIAnimationParameter::Get().Load("Assets/ui/uiAnimation/UIAnimation.json");

	//layout = new Layout();
	//layout->Initialize<SoundOptionMenu>("Assets/ui/layout/SoundOptionMenu.json");
}


InGameUIManager::~InGameUIManager()
{
}


void InGameUIManager::Update()
{
}