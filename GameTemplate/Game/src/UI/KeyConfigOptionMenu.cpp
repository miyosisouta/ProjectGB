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
}


void KeyConfigOptionMenu::Update()
{
	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());

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


	// デフォルト
	if (selectKeyConfigType == DEFAULT_KEY_TYPE) {

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


	// 選択中
	{
		// 黄色の背景
		auto* selectFrame = GetUI<UIIcon>(Hash32("flame"));
		// 青緑の枠
		auto* selectFrame2 = GetUI<UIIcon>(Hash32("flame2"));
		float posY = 200.0f;
		float offsetY = 200.0f;
		switch (selectKeyConfigType)
		{
		case SKILL_KEY_TYPE:
		{
			posY = 200.0f;
			selectFrame->isDraw = true;
			selectFrame2->isDraw = true;
			break;
		}
		case RUN_KEY_TYPE:
		{
			posY = 200.0f - offsetY;
			selectFrame->isDraw = true;
			selectFrame2->isDraw = true;
			break;
		}
		case ATTACK_KEY_TYPE:
		{
			posY = 200.0f - (offsetY * 2.0f);
			selectFrame->isDraw = true;
			selectFrame2->isDraw = true;
			break;
		}
		default:
		{
			selectFrame->isDraw = false;
			selectFrame2->isDraw = false;
			break;
		}
		}
		selectFrame->transform.localPosition.y = posY;
		selectFrame2->transform.localPosition.y = posY;
	}

	// 選択されたボタンのみを画像切り替え
	{
		UpdateSelectedButton(enButtonA);
		UpdateSelectedButton(enButtonB);
		UpdateSelectedButton(enButtonX);
		UpdateSelectedButton(enButtonY);
	}

	// 押されたボタンの表示
	{
		UpdateButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
		UpdateButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
		UpdateButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);
	}

	{
		
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

