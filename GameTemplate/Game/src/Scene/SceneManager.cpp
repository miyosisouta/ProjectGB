/**
 * SceneManager.cpp
 * 
 * シーンの遷移を管理
 */

#include "stdafx.h"
#include "SceneManager.h"
#include "src/Scene/IScene.h"
#include "src/Scene/InGameScene.h"
#include "src/Scene/OutGameScene.h"


SceneManager* SceneManager::myInstance_ = nullptr;


SceneManager::SceneManager()
{
	AddSceneMap <IScene>();
	AddSceneMap <InGameScene>();
	AddSceneMap <OutGameScene>();
}


SceneManager::~SceneManager()
{

}


void SceneManager::Update()
{
	//現行シーンの更新
	if (currentScene_) currentScene_->Update();
}


void SceneManager::CreateScene(const uint32_t id)
{
	auto it = sceneMap_.find(id);
	if (it == sceneMap_.end()) {
		K2_ASSERT(false, "新規シーンが追加されていません。\n");
	}
	auto& createSceneFunc = it->second;
	currentScene_ = createSceneFunc();
	currentScene_->Start();
}