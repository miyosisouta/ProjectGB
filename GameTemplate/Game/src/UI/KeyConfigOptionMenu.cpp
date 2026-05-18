/**
 * KeyConfigOptionMenu.cpp
 * ゲームクリア時の演出等用
 */
#include "stdafx.h"
#include "KeyConfigOptionMenu.h"
#include "UIAnimationFactory.h"

#include "src/sound/SoundManager.h"


namespace
{
	static int selectKeyConfigType = 0;
	static constexpr int SKILL_KEY_TYPE = 0;
	static constexpr int RUN_KEY_TYPE = 1;
	static constexpr int ATTACK_KEY_TYPE = 2;
	static constexpr int DEFAULT_KEY_TYPE = 3;

	// どのボタンが押されたか
	bool IsInputTargetKey()
	{
		if (g_pad[0]->IsTrigger(enButtonA)) {
			return true;
		}
		if (g_pad[0]->IsTrigger(enButtonB)) {
			return true;
		}
		if (g_pad[0]->IsTrigger(enButtonX)) {
			return true;
		}
		if (g_pad[0]->IsTrigger(enButtonY)) {
			return true;
		}

		return false;
	}

}




void KeyConfigOptionMenu::Update()
{
	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());

	


	// デフォルト
	if (selectKeyConfigType == DEFAULT_KEY_TYPE) {
		if (g_pad[0]->IsTrigger(enButtonA)) {
			/*skillButton = DEFAULT_KEY_TYPE;
			runButton = DEFAULT_KEY_TYPE;
			attackButton = DEFAULT_KEY_TYPE;*/

			KeyConfig::Get().SetKeyBind(EnGameAction::enActionSpecialSkill,enButtonY);
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionDash,enButtonA);
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionNormalSkill, enButtonB);

			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);
		}


		// 選択中の時にでかくなる
		auto* buttonCanvs = GetUI<UICanvas>(Hash32("DefaultSoundIcon"));
		buttonCanvs->transform.localScale = Vector3(1.2f);
		// 背景の色が変わる
		auto* normalBack = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_back"));
		auto* selectColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/BackSelectColorDummy"));
		if (normalBack) {
			normalBack->color = selectColorDummy->color;
		}
		// 枠の色が変わる
		auto* nomalFlame = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_flame"));
		auto* selectFlameColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/FlameSelectColorDummy"));
		if (nomalFlame) {	
			nomalFlame->color = selectFlameColorDummy->color;
		}
	}
	else {
		auto* buttonCanvs = GetUI<UICanvas>(Hash32("DefaultSoundIcon"));
		buttonCanvs->transform.localScale = Vector3(1.0f);
		// 背景の色が変わる
		auto* normalBack = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_back"));
		auto* normalColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/BackNormalColorDummy"));
		normalBack->color = normalColorDummy->color;
		// 枠の色が変わる
		auto* nomalFlame = GetUI<UIIcon>(Hash32("DefaultSoundIcon/Button_Default_flame"));
		auto* selectFlameColorDummy = GetUI<UIDummy>(Hash32("DefaultSoundIcon/FlameNormalColorDummy"));
		nomalFlame->color = selectFlameColorDummy->color;
	}


	

	// @todo for test 一旦、分けてみる
	switch (keyConfigState)
	{
	case KEY_CONFIG_STATE_SELECT:

		// 選択の青枠
		UpdateSelectFlame();

		if (g_pad[0]->IsTrigger(enButtonA))
		{
			// 青枠点滅：光
			auto* selectFlame = GetUI<UIIcon>(Hash32("flame"));
			auto* animation = selectFlame->FindAnimation(Hash32("KeyConfig_selectFlame_FadeIn"));
			animation->Clear();
			animation->Play();
			
			isWaiting = true;

			// 状態の切り替え
			keyConfigState = KEY_CONFIG_STATE_WAIT_ACTION;
		}
		break;

	case KEY_CONFIG_STATE_WAIT_ACTION:
		if (IsInputTargetKey())
		{
			// 選択されたボタンのみを画像切り替え
			{
				UpdateSelectedButton(enButtonA);
				UpdateSelectedButton(enButtonB);
				UpdateSelectedButton(enButtonX);
				UpdateSelectedButton(enButtonY);
			}

			// 状態の切り替え
			keyConfigState = KEY_CONFIG_STATE_DECIDE;
		}
		break;

	case KEY_CONFIG_STATE_DECIDE:
		isWaiting = false;

		// 青枠点滅：消
		auto* selectFlame = GetUI<UIIcon>(Hash32("flame"));
		auto* dummyFlame = GetUI<UIDummy>(Hash32("InitFlame"));
		auto* animation = selectFlame->FindAnimation(Hash32("KeyConfig_selectFlame_FadeIn"));
		animation->Stop();
		selectFlame->color.w = dummyFlame->color.w;

		// 状態の切り替え
		keyConfigState = KEY_CONFIG_STATE_SELECT;

		break;
	}

	// 押されたボタンの表示
	{
		switch (selectKeyConfigType)
		{
		case SKILL_KEY_TYPE:
			UpdateButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
			break;

		case RUN_KEY_TYPE:
			UpdateButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
			break;

		case ATTACK_KEY_TYPE:
			UpdateButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);
			break;
		}
	}	

	MenuBase::Update();
}


void KeyConfigOptionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void KeyConfigOptionMenu::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();

	// 一回だけよぶアタッチ
	auto* selectFlame = GetUI<UIIcon>(Hash32("flame"));
	UIAnimationFactory::Attach<UIColorAnimation>(selectFlame, Hash32("KeyConfig_selectFlame_FadeIn"));

	const int id = taskScheduler->CreateLoopSequence(20.0f);
	{
		nikukyuList[0] = GetUI<UIIcon>(Hash32("optionBack/nikukyu1"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[0], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 0.0f, [&]()
			{
				auto* animation = nikukyuList[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 6.0f, [&]()
			{
				auto* animation = nikukyuList[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList[1] = GetUI<UIIcon>(Hash32("optionBack/nikukyu2"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[1], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 4.0f, [&]()
			{
				auto* animation = nikukyuList[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 10.0f, [&]()
			{
				auto* animation = nikukyuList[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList[2] = GetUI<UIIcon>(Hash32("optionBack/nikukyu3"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[2], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 8.0f, [&]()
			{
				auto* animation = nikukyuList[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 14.0f, [&]()
			{
				auto* animation = nikukyuList[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList[3] = GetUI<UIIcon>(Hash32("optionBack/nikukyu4"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList[3], Hash32("nikukyuu_fadein"));
		taskScheduler->AddLoopTimer(id, 12.0f, [&]()
			{
				auto* animation = nikukyuList[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler->AddLoopTimer(id, 18.0f, [&]()
			{
				auto* animation = nikukyuList[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}

	{
		UpdateButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
		UpdateButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
		UpdateButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);
	}
}

void KeyConfigOptionMenu::UpdateSelectFlame()
{
	// キーの種類選択
	{
		if (g_pad[0]->IsTrigger(enButtonUp)) {
			selectKeyConfigType--;
			if (selectKeyConfigType < SKILL_KEY_TYPE) {
				selectKeyConfigType = SKILL_KEY_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonDown)) {
			selectKeyConfigType++;
			if (selectKeyConfigType > DEFAULT_KEY_TYPE) {
				selectKeyConfigType = DEFAULT_KEY_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}

	// 青緑の枠
	auto* selectFrame = GetUI<UIIcon>(Hash32("flame"));
	float posY = 200.0f;
	float offsetY = 200.0f;
	switch (selectKeyConfigType)
	{
		case SKILL_KEY_TYPE:
		{
			posY = 200.0f;
			selectFrame->isDraw = true;
			break;
		}
		case RUN_KEY_TYPE:
		{
			posY = 200.0f - offsetY;
			selectFrame->isDraw = true;
			break;
		}
		case ATTACK_KEY_TYPE:
		{
			posY = 200.0f - (offsetY * 2.0f);
			selectFrame->isDraw = true;
			break;
		}
		default:
		{
			selectFrame->isDraw = false;
			break;
		}
	}
	selectFrame->transform.localPosition.y = posY;
}

void KeyConfigOptionMenu::UpdateSelectedButton(const int buttonType)
{
	if (g_pad[0]->IsTrigger(static_cast<EnButton>(buttonType))) {
		switch (selectKeyConfigType)
		{
			case SKILL_KEY_TYPE:
			{
				KeyConfig::Get().SetKeyBind(enActionSpecialSkill, buttonType);
				break;
			}
			case RUN_KEY_TYPE:
			{
				KeyConfig::Get().SetKeyBind(enActionDash, buttonType);
				break;
			}
			case ATTACK_KEY_TYPE:
			{
				KeyConfig::Get().SetKeyBind(enActionNormalSkill, buttonType);
				break;
			}
		}
	}
}

void KeyConfigOptionMenu::UpdateButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int actionKey)
{
	// 非表示
	auto* buttonA = GetUI<UIIcon>(buttonAId);
	buttonA->isDraw = false;
	auto* buttonB = GetUI<UIIcon>(buttonBId);
	buttonB->isDraw = false;
	auto* buttonX = GetUI<UIIcon>(buttonXId);
	buttonX->isDraw = false;
	auto* buttonY = GetUI<UIIcon>(buttonYId);
	buttonY->isDraw = false;


	// もし、ボタンが押されなかったら抜ける
	if(keyConfigState == KEY_CONFIG_STATE_WAIT_ACTION && isWaiting)
	{
		return;
	}

	// 初期キーを変数に入れる
	int attackKey = KeyConfig::Get().GetBindButton(static_cast<EnGameAction>(actionKey));
	// 特定のボタンが押されたら表示
	if (attackKey == enButtonA) {
		buttonA->isDraw = true;
	}
	if (attackKey == enButtonB) {
		buttonB->isDraw = true;
	}
	if (attackKey == enButtonX) {
		buttonX->isDraw = true;
	}
	if (attackKey == enButtonY) {
		buttonY->isDraw = true;
	}
}

