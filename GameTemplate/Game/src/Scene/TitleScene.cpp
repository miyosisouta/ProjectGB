/**
 * TitleScene.h
 *
 * タイトルシーンの遷移
 */

#include "stdafx.h"
#include "TitleScene.h"

#include "src/Scene/InGameScene.h"
#include "src/Scene/TitleBackground.h"
#include "src/Sound/SoundManager.h"

#include "src/UI/Layout.h"
#include "src/UI/TitleMenu.h"
#include "src/UI/LoadingMenu.h"
#include "src/UI/BossSelectMenu.h"
#include "src/UI/SkillSelectMenu.h"
#include "src/UI/CheckStartWindow.h"
#include "src/UI/OptionMenu.h"
#include "src/UI/SoundOptionMenu.h"
#include "src/UI/KeyConfigOptionMenu.h"
#include "src/UI/WarningButtonWindow.h"
#include "src/UI/CameraOptionMenu.h"
#include "src/UI/PlayGameMenu.h"
#include "src/UI/UIScreenManager.h"

namespace
{
	constexpr float DOG_RUN_WAIT_TIME = 1.5f; // Loading前に犬が走っていく時間
	constexpr float LOADING_TIME = 3.0f; // ローディング画面の時間
}

TitleScene::TitleScene()
{
    titleBackground_ = NewGO<TitleBackground>(0, "titleBackground");
}


TitleScene::~TitleScene()
{
    DeleteGO(titleBackground_);
}


bool TitleScene::Start()
{
	UIScreenManager::Get().Boot<TitleMenu>("Assets/ui/layout/TitleMenu.json");
	SoundManager::Get().PlayBGM(enSoundKind_Title);
	return true;
}


void TitleScene::Update()
{
	auto* menu = UIScreenManager::Get().GetActiveMenu();

	// -------------------------------------------------------
	// タイトルメニュー
	// -------------------------------------------------------
	auto* titleMenu = dynamic_cast<TitleMenu*>(menu);
	if (titleMenu) {
		// 遷移してもよいか
		if (playGameMenuWasActive_ && !UIScreenManager::Get().IsTransitioning()) {
			playGameMenuWasActive_ = false;
			titleBackground_->SetScrollEnabled(true); // タイトルのスクロール開始
		}
		if (optionMenuWasActive_ && !UIScreenManager::Get().IsTransitioning()) {
			optionMenuWasActive_ = false;
			titleBackground_->SetScrollEnabled(true); // タイトルのスクロール開始
		}

		// 遷移中はボタン入力を受け付けない
		if (pendingLoad_ == PendingLoad::None && titleMenu->IsAbuttonEnabled()) {

			// はじめるを選んでいる場合
			if (titleMenu->IsSelectStat()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					// タイトルUIを即座に非表示 → 犬を走らせる
					if (auto* c = titleMenu->GetCanvas()) c->color.w = 0.0f;
					titleMenu->StopEffect(); // 星のキラキラエフェクトも消す
					titleBackground_->StartPlayerRunOff();
					pendingLoad_ = PendingLoad::WaitingForDog;
				}
			}

			// あそびかたを選んでいる場合
			if (titleMenu->IsSelectPlay()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					titleBackground_->SetScrollEnabled(false); // タイトルのスクロール停止
					playGameMenuWasActive_ = true; // オプションメニューを開く
					UIScreenManager::Get().Push<PlayGameMenu>("Assets/ui/layout/PlayGameMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
				}
			}

			// せっていを選んでいる場合
			if (titleMenu->IsSelectOption()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					titleBackground_->SetScrollEnabled(false); // タイトルのスクロール停止
					optionMenuWasActive_ = true; // オプションメニューを開く
					UIScreenManager::Get().Push<OptionMenu>("Assets/ui/layout/OptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
				}
			}

			// おわるを選んでいる場合
			if (titleMenu->IsSelectExit()) {
				if (g_pad[0]->IsTrigger(enButtonA)) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
					exit(0);
				}
			}
		}

		// 犬が走り去るのを待ち Loading を開始
		if (pendingLoad_ == PendingLoad::WaitingForDog) {
			loadingTimer_ += g_gameTime->GetFrameDeltaTime();
			// 犬が一定時間走ったら、
			if (loadingTimer_ >= DOG_RUN_WAIT_TIME) {
				loadingTimer_ = 0.0f;
				pendingLoad_ = PendingLoad::ToBossSelect; // ボスセレクトに遷移
				// 先にローディングを開始
				UIScreenManager::Get().Push<LoadingMenu>(
					"Assets/ui/layout/LoadingMenu.json",
					UITransitionMode::Replace,
					UIScreenTransitionPreset::FadeInOut());
			}
		}
	}

	// -------------------------------------------------------
	// ローディング
	// Loading が完全に表示されたら BossSelect 画面に遷移する。
	// BossSelect が Loading(Inactive) を背景にしてフェードインするため
	// チラツキがなくなる。フェードイン完了後に Inactive の Loading を削除する。
	// -------------------------------------------------------
	auto* loadingMenu = dynamic_cast<LoadingMenu*>(menu);
	if (loadingMenu && !UIScreenManager::Get().IsTransitioning()) {
		loadingTimer_ += g_gameTime->GetFrameDeltaTime();
		if (pendingLoad_ == PendingLoad::ToBossSelect && loadingTimer_ >= LOADING_TIME) {
			loadingTimer_ = 0.0f;
			pendingLoad_ = PendingLoad::None;
			UIScreenCallbacks bossCb;
			bossCb.onActive = [this]() {
				// BossSelect のフェードイン完了後 → 非表示中 Loading を削除
				UIScreenManager::Get().ClearInactive();
			};
			UIScreenManager::Get().Push<BossSelectMenu>(
				"Assets/ui/layout/BossSelectMenu.json",
				UITransitionMode::Push,
				UIScreenTransitionPreset::FadeInOut(),
				bossCb);
		}
		// Loading が完全に表示されたら TitleMenu に遷移する。
		// Loading がまだ不透明な間に背景をリセットするため、チラツキがなくなる。
		if (pendingLoad_ == PendingLoad::ToTitleMenu && loadingTimer_ >= LOADING_TIME) {
			loadingTimer_ = 0.0f;
			pendingLoad_ = PendingLoad::None;
			titleBackground_->ResetBackground(); // Loading が不透明な間にリセット
			UIScreenCallbacks titleCb;
			titleCb.onActive = [this]() {
				// TitleMenu のフェードイン完了後 → 非表示中の Loading を削除
				UIScreenManager::Get().ClearInactive();
			};
			UIScreenManager::Get().Push<TitleMenu>(
				"Assets/ui/layout/TitleMenu.json",
				UITransitionMode::Replace,
				UIScreenTransitionPreset::FadeInOut(),
				titleCb);
		}
	}

	// -------------------------------------------------------
	// ボスセレクトメニュー
	// -------------------------------------------------------
	auto* bossSelectMenu = dynamic_cast<BossSelectMenu*>(menu);

	if (bossSelectMenu) {
		// ボス選択したら
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonA)) {
			// スキル選択画面へ
			UIScreenManager::Get().Push<SkillSelectMenu>("Assets/ui/layout/SkillSelectMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			
			// 初期化
			std::string stageKey = "Gorilla";

			// ゴリラを選択したら
			if (bossSelectMenu->IsSelectGollira()) {
				stageKey = "Gorilla";
			}
			// カメを選択したら
			else if (bossSelectMenu->IsSelectTurtle()) {
				stageKey = "Turtle";
			}

			// ボスのkeyをデータベースへ設定
			CharacterDataBase::Get().SetStageKey(stageKey);
			// TODO: チュートリアルを行うか選ぶUI画面が実装されたら、その決定処理で
			//       CharacterDataBase::Get().SetTutorialEnabled(選択結果) を呼んでください（未設定時はtrue＝チュートリアルを行う）
			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
		}
	}

	// タイトルへ戻る（BossSelect の上に Loading を重ねてからチラツキを防ぐ）
	if (bossSelectMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			pendingLoad_ = PendingLoad::ToTitleMenu;
			loadingTimer_ = 0.0f;
			// Push で BossSelect の上に Loading をフェードインさせる。
			// Replace だと BossSelect が先に消え、背後の TitleMenu(Inactive) が一瞬見えてしまうため。
			UIScreenCallbacks loadingCb;
			loadingCb.onActive = [this]() {
				// Loading が完全に表示されてから BossSelect を削除する
				UIScreenManager::Get().ClearInactive();
			};
			UIScreenManager::Get().Push<LoadingMenu>(
				"Assets/ui/layout/LoadingMenu.json",
				UITransitionMode::Push,
				UIScreenTransitionPreset::FadeInOut(),
				loadingCb);
		}
	}

	// -------------------------------------------------------
	// スキル選択
	// -------------------------------------------------------
	auto* skillSelectMenu = dynamic_cast<SkillSelectMenu*>(menu);
	if (skillSelectMenu) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			AbilityType skillType = AbilityType::enDefault;
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
			UIScreenManager::Get().Push<CheckStartWindow>("Assets/ui/layout/CheckStartWindow.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
		}
	}
	if (skillSelectMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}

	// -------------------------------------------------------
	// 確認ウィンドウ
	// -------------------------------------------------------
	auto* checkStartWindow = dynamic_cast<CheckStartWindow*>(menu);
	if (checkStartWindow) {
		if (!UIScreenManager::Get().IsTransitioning()) {
			if (g_pad[0]->IsTrigger(enButtonB)) {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
				UIScreenManager::Get().Pop();
			}
			if (g_pad[0]->IsTrigger(enButtonA)) {
				if (checkStartWindow->IsSelectBackButton()) {
					SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
					UIScreenManager::Get().Pop();
				}
				else if (checkStartWindow->IsSelectStartButton()) {
					isRequestScene = true;
					UIScreenManager::Get().AllPop();
				}
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
			}
		}
	}


	// -------------------------------------------------------
	// あそびかた
	// -------------------------------------------------------
	auto* playMenu = dynamic_cast<PlayGameMenu*>(menu);
	if (playMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}


	// -------------------------------------------------------
	// オプション系
	// -------------------------------------------------------
	auto* soundMenu = dynamic_cast<SoundOptionMenu*>(menu);
	if (soundMenu) {
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}

	auto* optionMenu = dynamic_cast<OptionMenu*>(menu);
	if (optionMenu) {
		if (optionMenu->IsSelectSound() && g_pad[0]->IsTrigger(enButtonA)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
			UIScreenManager::Get().Push<SoundOptionMenu>("Assets/ui/layout/SoundOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
		}
		if (optionMenu->IsSelectKeyConfig() && g_pad[0]->IsTrigger(enButtonA)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
			UIScreenManager::Get().Push<KeyConfigOptionMenu>("Assets/ui/layout/KeyConfigOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
		}
		if (optionMenu->IsSelectCamera() && g_pad[0]->IsTrigger(enButtonA)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
			UIScreenManager::Get().Push<CameraOptionMenu>("Assets/ui/layout/CameraOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
		}
		if (!UIScreenManager::Get().IsTransitioning() && g_pad[0]->IsTrigger(enButtonB)) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
			UIScreenManager::Get().Pop();
		}
	}

	auto* keyConfigOptionMenu = dynamic_cast<KeyConfigOptionMenu*>(menu);
	if (keyConfigOptionMenu) {
		if (keyConfigOptionMenu->IsButtonOverLap()) {
			if (!KeyConfigOptionMenu::IsWarningWindowCancel() && !KeyConfigOptionMenu::IsWarningWindowClose()) {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
				UIScreenManager::Get().Push<WarningButtonWindow>("Assets/ui/layout/WarningButtonWindow.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());
			}
		}
		if (!UIScreenManager::Get().IsTransitioning() && !keyConfigOptionMenu->IsButtonSetting()) {
			if (g_pad[0]->IsTrigger(enButtonB)) {
				KeyConfigOptionMenu::SetWarningWindowCancel(true);
				SoundManager::Get().PlaySE(enSoundKind_Menu_Return);
				UIScreenManager::Get().Pop();
			}
		}
	}

	auto* warningButtonWindow = dynamic_cast<WarningButtonWindow*>(menu);
	if (warningButtonWindow) {
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
	UIScreenManager::Get().Render(rc);
}


bool TitleScene::RequestScene(uint32_t& id)
{
	if (isRequestScene) {
		id = InGameScene::ID();
		SoundManager::Get().StopBGM();
		return true;
	}
	return false;
}