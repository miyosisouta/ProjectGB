/**
 * BootScene.cpp
 *
 * タイトル画面の描画
 */

#include "stdafx.h"
#include "StartupScene.h"
#include "DebugScene.h"
#include "DebugGrassScene.h"
#include "BootScene.h"
#include "TitleScene.h"
#include "InGameScene.h"
#include "OutGameScene.h"

#include "src/sound/SoundManager.h"
#include "src/UI/BootMenu.h"



namespace {
	struct SelectSceneInformation
	{
		const wchar_t* name;
		Vector3 position;
		uint32_t sceneId;
		//
		SelectSceneInformation(const wchar_t* name, const Vector3& pos, const uint32_t id)
			: name(name)
			, position(pos)
			, sceneId(id)
		{
		}
	};

	static const SelectSceneInformation selectSceneinformationList[enSceneKind_Max] =
	{
		SelectSceneInformation(L"ブート", Vector3(0.0f,200.0f,0.0f), BootScene::ID()),
		SelectSceneInformation(L"タイトル", Vector3(0.0f,100.0f,0.0f), TitleScene::ID()),
		SelectSceneInformation(L"ゲーム", Vector3(0.0f,0.0f,0.0f), InGameScene::ID()),
		SelectSceneInformation(L"デバック", Vector3(0.0f,-100.0f,0.0f), DebugScene::ID()),
		SelectSceneInformation(L"デバック_Grass", Vector3(0.0f,-200.0f,0.0f), DebugGrassScene::ID()),
	};
	
	static const Vector3 SELECTOR_POSITION_LIST[enSceneKind_Max] =
	{
		Vector3(-50.0f,190.0f,0.0f),
		Vector3(-50.0f,80.0f,0.0f),
		Vector3(-50.0f,-20.0f,0.0f),
		Vector3(-50.0f,-120.0f,0.0f),
		Vector3(-50.0f,-220.0f,0.0f),
	};
}


StartupScene::StartupScene()
{
}


StartupScene::~StartupScene()
{
}


bool StartupScene::Start()
{
	// シーン選択用に表示、どんなシーンがあるか
	for (int i = 0; i < enSceneKind_Max; i++) {
		const auto& info = selectSceneinformationList[i];
		sceneText_[i].SetText(info.name);
		sceneText_[i].SetPosition(info.position);
	}
	// 選択している箇所を分かるようにするアイコン
	selecterRender_.Init("Assets/ui/titleUI/nikukyu.DDS",64.0f,64.0f);

	return true;
}


void StartupScene::Update()
{
	if (g_pad[0]->IsTrigger(enButtonUp)) {
		selectIndex_--;
		// 最小値チェック
		if (selectIndex_ < enSceneKind_Default) {
			selectIndex_ = enSceneKind_Default;	// 範囲外にならないように調整
		}
	}
	else if (g_pad[0]->IsTrigger(enButtonDown)) {
		selectIndex_++;
		// 最大値チェック
		if (selectIndex_ >= enSceneKind_Max) {
			selectIndex_ = enSceneKind_Max - 1;	// 範囲外にならないように調整
		}
	}

	// シーン設定
	if (g_pad[0]->IsTrigger(enButtonA)) {
		const auto& info = selectSceneinformationList[selectIndex_];
		requestSceneId_ = info.sceneId;
	}

	selecterRender_.SetPosition(SELECTOR_POSITION_LIST[selectIndex_]);
	selecterRender_.Update();
}


void StartupScene::Render(RenderContext& rc)
{
	// シーンの名前描画
	for (int i = 0; i < enSceneKind_Max; i++) {
		sceneText_[i].Draw(rc);
	}
	// セレクター描画
	selecterRender_.Draw(rc);
}


bool StartupScene::RequestScene(uint32_t& id)
{
	if (requestSceneId_ != INVALID_SCENE_ID) {
		id = requestSceneId_;
		return true;
	}
	return false;
}