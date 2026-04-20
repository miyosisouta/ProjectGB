/**
 * InGameScene.cpp
 *
 * インゲームシーンの遷移
 */

#include "stdafx.h"
#include "InGameScene.h"

#include "src/Battle/BattleManager.h"
#include "src/Core/PouseMenu.h"
#include "src/Scene/OutGameScene.h"
#include "src/Scene/TitleScene.h"
#include "src/Sound/SoundManager.h"

#include "src/UI/Layout.h"
#include "src/UI/InGameMenu.h"


InGameScene::InGameScene()
{
	BattleManager::CreateInstance();		//BattleManagerのインスタンスを作成
	
	pouseMenu_ = NewGO<PouseMenu>(0, "pouseMenu");	//PouseMenuの生成

}


InGameScene::~InGameScene()
{
	BattleManager::Get().DestroyInstance();		//BattleManagerのインスタンスを破棄	
	delete layout_;
	DeleteGO(pouseMenu_);
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


	// ポーズメニューの呼び出し
	{
		if (g_pad[0]->IsTrigger(enButtonStart)) {			

			// ポーズメニューの開閉状態を切り替える
			isOpenPauseMenu_ = !isOpenPauseMenu_;

			if (isOpenPauseMenu_) {

				// 更新対象をUIのみに切り替える
				activeTarget_ = UpdateGroup::UI;
				BattleManager::Get().SetActiveTarget(activeTarget_);

				// 時間を停止させる
				BattleManager::Get().SetPouse(true);

				// ポーズメニューを開く
				pouseMenu_->SetActive(true);
			}
			else {
				// 更新対象を全てに切り替える
				activeTarget_ = UpdateGroup::All;
				BattleManager::Get().SetActiveTarget(activeTarget_);
				BattleManager::Get().SetPouse(false);

				// ポーズメニューを閉じる
				pouseMenu_->SetActive(false);
			}
		}
	}	
}


void InGameScene::Render(RenderContext& rc)
{
	BattleManager::Get().Render(rc);
	if (BattleManager::Get().IsCutScene()) {
		return;
	}
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

	// ポーズ画面からタイトルへ戻る
	if (pouseMenu_->IsReturnTitle() == true) {
		id = TitleScene::ID();
		SoundManager::Get().StopBGM();
	 
		return true;
	}
	// ゲーム演出終了からタイトルへもどる
	if (BattleManager::Get().IsFinishedGame()) {
		id = TitleScene::ID();
		SoundManager::Get().StopBGM();

		return true;
	}

	return false;
}