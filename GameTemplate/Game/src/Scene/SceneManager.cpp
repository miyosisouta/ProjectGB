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
#include "src/Scene/LoadingScreen.h"


SceneManager* SceneManager::myInstance_ = nullptr;


SceneManager::SceneManager()
{
	// 各シーンを配列に追加
	AddSceneMap <InGameScene>();
	AddSceneMap <OutGameScene>();

	//ロード画面クラスを生成
	loadingScreen_ = NewGO<LoadingScreen>(0, "LoadingScreen");

	//最初のシーンを生成（アウトゲームシーン）
	CreateScene(OutGameScene::ID());
}


SceneManager::~SceneManager()
{
	//ロード画面クラスを削除
	DeleteGO(loadingScreen_);
}


void SceneManager::Update()
{
	//現行シーンの更新
	if (currentScene_) {
		currentScene_->Update();
		if (currentScene_->RequestScene(requestSceneID_)) {

			//ロード画面表示のフラグをたてる
			loadingScreen_->SetDraw(true);

			//次のシーンへ
			delete currentScene_;
			CreateScene(requestSceneID_);

			//ロード画面表示のフラグをおろす
			loadingScreen_->SetDraw(false);
		}
	}

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


/******************************************************************************************************/


SceneMangerObject::SceneMangerObject()
{
	//シーンマネージャーを生成
	SceneManager::CreateInstance();
}


SceneMangerObject::~SceneMangerObject()
{
	//シーンマネージャーを削除
	SceneManager::DeleteInstance();
}


bool SceneMangerObject::Start()
{
	return true;
}


void SceneMangerObject::Update()
{
	SceneManager::GetInstance()->Update();
}