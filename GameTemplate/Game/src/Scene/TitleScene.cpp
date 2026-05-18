/**
 * TitleScene.h
 *
 * タイトルシーンの遷移
 */

#include "stdafx.h"
#include "TitleScene.h"

#include "src/Scene/InGameScene.h"
#include "src/Scene/TitleScreen.h"
#include "src/Sound/SoundManager.h"

#include "src/UI/Layout.h"
#include "src/UI/TitleMenu.h"
#include "src/UI/OptionMenu.h"
#include "src/UI/SoundOptionMenu.h"
#include "src/UI/KeyConfigOptionMenu.h"
#include "src/UI/UIScreenManager.h"


TitleScene::TitleScene()
{
}


TitleScene::~TitleScene()
{
}


bool TitleScene::Start()
{
	UIScreenManager::Get().Boot<TitleMenu>("Assets/ui/layout/TitleMenu.json");

	// タイトルBGM再生
	SoundManager::Get().PlayBGM(enSoundKind_Title);

	return true;
}


void TitleScene::Update()
{
	auto* menu = UIScreenManager::Get().GetActiveMenu();
	auto* titleMenu = dynamic_cast<TitleMenu*>(menu);
	// タイトルのメニューが有効な時の処理
	if (titleMenu) {
		if (titleMenu->IsAbuttonEnabled()) {
			if (titleMenu->IsSelectStat()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					isRequestScene = true;

					UIScreenManager::Get().Pop();
				}
			}
			if (titleMenu->IsSelectOption()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					UIScreenManager::Get().Push<OptionMenu>("Assets/ui/layout/OptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
				}
			}
			if (titleMenu->IsSelectExit()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {//
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					exit(0);
				}
			}
		}
	}
	// サウンドメニューが有効な時
	auto* soundMenu = dynamic_cast<SoundOptionMenu*>(menu);
	if (soundMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}


	// オプションメニューの遷移(音の設定・キーの設定・カメラの設定)
	auto* optionMenu = dynamic_cast<OptionMenu*>(menu);
	if (optionMenu)
	{
		if (optionMenu->IsSelectSound()) {
			if (g_pad[0]->IsTrigger(enButtonA)) {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
				UIScreenManager::Get().Push<SoundOptionMenu>("Assets/ui/layout/SoundOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			}
		}
		if (optionMenu->IsSelectKeyConfig()) {
			if (g_pad[0]->IsTrigger(enButtonA)) {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
				UIScreenManager::Get().Push<KeyConfigOptionMenu>("Assets/ui/layout/KeyConfigOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			}
		}
		if (optionMenu->IsSelectCamera()) {
			if (g_pad[0]->IsTrigger(enButtonA)) {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
				UIScreenManager::Get().Push<SoundOptionMenu>("Assets/ui/layout/SoundOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			}
		}
	}

	// オプションメニューが有効な時 戻る
	//auto* optionMenu = dynamic_cast<OptionMenu*>(menu);
	if (optionMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}

	UIScreenManager::Get().Update();
}


void TitleScene::Render(RenderContext& rc)
{
	//layout_->Render(rc);
	UIScreenManager::Get().Render(rc);
}


bool TitleScene::RequestScene(uint32_t& id)
{
	//Aボタンが押されたら次のシーンへ（仮）
	if(isRequestScene) {
		id = InGameScene::ID();

		// タイトルBGMストップ
		SoundManager::Get().StopBGM();

		return true;
	}

	return false;
}