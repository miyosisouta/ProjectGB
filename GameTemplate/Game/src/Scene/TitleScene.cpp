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
#include "src/UI/BossSelectMenu.h"
#include "src/UI/SkillSelectMenu.h"
#include "src/UI/CheckStartWindow.h"
#include "src/UI/OptionMenu.h"
#include "src/UI/SoundOptionMenu.h"
#include "src/UI/KeyConfigOptionMenu.h"
#include "src/UI/WarningButtonWindow.h"
#include "src/UI/CameraOptionMenu.h"
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
					/*SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					isRequestScene = true;

					UIScreenManager::Get().Pop();*/
					// すぐ閉じる
					UIScreenManager::Get().Push<BossSelectMenu>("Assets/ui/layout/BossSelectMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::None());
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


	// ボス選択メニューが有効な時(ゴリラorカメを選択)
	auto* bossSelectMenu = dynamic_cast<BossSelectMenu*>(menu);

	if (bossSelectMenu) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			// スキル選択画面
			UIScreenManager::Get().Push<SkillSelectMenu>("Assets/ui/layout/SkillSelectMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			// ボスを選択
			BossType stageType = BossType::enGorilla;
			GameModeType mode = GameModeType::enNormal;
			// 選択されたボスになる
			if (bossSelectMenu->IsSelectGollira()) {
				stageType = BossType::enGorilla;
			}
			else if (bossSelectMenu->IsSelectTurtle()) {
				stageType = BossType::enTurtle;
			}

			CharacterDataBase::Get().SetStageType(stageType);
			CharacterDataBase::Get().SetGameModeType(mode);

			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
			//isRequestScene = true;

			//UIScreenManager::Get().Pop();
		}
	}


	// ボス選択メニューが有効な時(タイトルに戻る)
	if (bossSelectMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}


	// スキル選択メニューが有効な時
	auto* skillSelectMenu = dynamic_cast<SkillSelectMenu*>(menu);

	if (skillSelectMenu) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			AbilityType skillType = AbilityType::enDefault;
			// アビリティの選択をする
			if (skillSelectMenu->IsSelectSkillLandMine()) {
				skillType = AbilityType::enLandmine;
			}
			else if (skillSelectMenu->IsSelectSkillBite()) {
				skillType = AbilityType::enDefault;
			}
			else if (skillSelectMenu->IsSelectSkillFire()) {
				skillType = AbilityType::enFireMagic;
			}

			CharacterDataBase::Get().SetPlayerAbility(skillType);

			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
			// 確認ウィンドウの表示
			UIScreenManager::Get().Push<CheckStartWindow>("Assets/ui/layout/CheckStartWindow.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			//isRequestScene = true;
			// 全部のUIを消す
			//UIScreenManager::Get().AllPop();
		}
	}

	// スキル選択メニューが有効な時(戻る)
	if (skillSelectMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}

	// 確認ウィンドウが有効な時
	auto* checkStartWindow = dynamic_cast<CheckStartWindow*>(menu);

	if (checkStartWindow) {
		if (!UIScreenManager::Get().IsTransitioning()) {
			if (g_pad[0]->IsTrigger(enButtonA)) {
				// 選択されたらスキル選択にもどる
				if (checkStartWindow->IsSelectBackButton()) {
					//CheckStartWindow::SetCheckWindowClose(true);
					SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
					UIScreenManager::Get().Pop();
				}
				// 選択されたらインゲーム
				else if (checkStartWindow->IsSelectStartButton()) {
					isRequestScene = true;
					UIScreenManager::Get().AllPop();
				}
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
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
				UIScreenManager::Get().Push<CameraOptionMenu>("Assets/ui/layout/CameraOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
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

	// キーコンフィグオプションメニューが有効な時
	auto* keyConfigOptionMenu = dynamic_cast<KeyConfigOptionMenu*>(menu);
	if (keyConfigOptionMenu) {
		// ボタンが重なったら
		if (keyConfigOptionMenu->IsButtonOverLap()) {
			// キャンセルした後なら処理したくない
			if (!KeyConfigOptionMenu::IsWarningWindowCancel() && !KeyConfigOptionMenu::IsWarningWindowClose()) {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
				UIScreenManager::Get().Push<WarningButtonWindow>("Assets/ui/layout/WarningButtonWindow.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			}
		}
		// キーコンフィグオプションメニューが有効な時 戻る
		if (!UIScreenManager::Get().IsTransitioning()) {
			if (!keyConfigOptionMenu->IsButtonSetting()) {
				if (g_pad[0]->IsTrigger(enButtonB)) {
					KeyConfigOptionMenu::SetWarningWindowCancel(true);
					SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
					UIScreenManager::Get().Pop();
				}
			}
		}
	}

	// 警告ウィンドウ(WarningButtonWindow)が有効な時
	auto* warningButtonWindow = dynamic_cast<WarningButtonWindow*>(menu);
	if (warningButtonWindow){
		if (!UIScreenManager::Get().IsTransitioning()) {
			if (g_pad[0]->IsTrigger(enButtonB)) {
				KeyConfigOptionMenu::SetWarningWindowCancel(true);
				SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
				UIScreenManager::Get().Pop();
			}
			if (g_pad[0]->IsTrigger(enButtonA)) {
				KeyConfigOptionMenu::SetWarningWindowClose(true);
				SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
				UIScreenManager::Get().Pop();
			}
		}
	}


	// カメラオプションメニューが有効な時 戻る
	auto* cameraOptionMenu = dynamic_cast<CameraOptionMenu*>(menu);
	if (cameraOptionMenu) {
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