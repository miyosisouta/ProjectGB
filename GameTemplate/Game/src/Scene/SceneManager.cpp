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
#include "src/Scene/titleScene.h"
#include "src/Scene/LoadingScreen.h"


SceneManager* SceneManager::myInstance_ = nullptr;


SceneManager::SceneManager()
{
	// 各シーンを配列に追加
	AddSceneMap <InGameScene>();
	AddSceneMap <OutGameScene>();
	AddSceneMap <TitleScene>();

	// ロード画面クラスを生成
	loadingScreen_ = NewGO<LoadingScreen>(100, "LoadingScreen");

	// 最初のシーンを生成（タイトルシーン）
	CreateScene(TitleScene::ID());

	
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

			if (!isNextScene_) {
				GnangeNextScene(requestSceneID_);
				isNextScene_ = true;				
			}			
		}
	}

	if (taskScheduler_) taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
}

void SceneManager::GnangeNextScene(const uint32_t id)
{

	// タスクシステムの生成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// スケジュール
	{
		// 1. ロード画面を描画させる
		taskScheduler_->AddTimer(0.1f, [&]()
			{
				loadingScreen_->StartDraw();
			});
		// 2. 次のシーンを生成
		taskScheduler_->AddTimer(1.5f, [&]()
			{
				delete currentScene_;
				CreateScene(requestSceneID_);
			});
		// 3. ロード画面描画を終了
		taskScheduler_->AddTimer(1.6f, [&]()
			{
				loadingScreen_->EndDraw();

				isNextScene_ = false;
			});
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