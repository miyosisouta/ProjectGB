/**
 * KeyConfigOptionMenu.cpp
 * ゲームクリア時の演出等用
 */
#include "stdafx.h"
#include "KeyConfigOptionMenu.h"
#include "UIAnimationFactory.h"

#include "src/sound/SoundManager.h"

// 警告ウィンドウ
#include "WarningButtonWindow.h"


namespace
{
	// 今選択されているキータイプ(ボタン設定)
	static int selectKeyConfigType = 0;
	static constexpr int SKILL_KEY_TYPE = 0;
	static constexpr int RUN_KEY_TYPE = 1;
	static constexpr int ATTACK_KEY_TYPE = 2;
	static constexpr int DEFAULT_KEY_TYPE = 3;

	// 現在設定されていないボタンの数
	static constexpr int MAX_KEY_TYPE_NUM = 3;

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

	// 押されたボタンがなにか
	EnButton GetInputButton()
	{

		if (g_pad[0]->IsTrigger(enButtonA)) {
			return enButtonA;
		}
		if (g_pad[0]->IsTrigger(enButtonB)) {
			return enButtonB;
		}
		if (g_pad[0]->IsTrigger(enButtonX)) {
			return enButtonX;
		}
		if (g_pad[0]->IsTrigger(enButtonY)) {
			return enButtonY;
		}

		return enButtonNum;
	}

	// ボタン設定がかぶっていたら(スキル：Aボタン 攻撃：Aボタン → true)
	EnButton GetChangeButton(const EnGameAction targetAction)
	{
		// 今設定されているボタン
		return static_cast<EnButton>(KeyConfig::Get().GetBindButton(targetAction));
	}

	EnGameAction GetSelectAction()
	{
		if (selectKeyConfigType == SKILL_KEY_TYPE) {
			return enActionSpecialSkill;
		}
		if (selectKeyConfigType == RUN_KEY_TYPE) {
			return enActionDash;
		}
		if (selectKeyConfigType == ATTACK_KEY_TYPE) {
			return enActionNormalSkill;
		}
	}

	bool FindOverlapButton(const EnButton inputButton, const EnGameAction selectAction)
	{
		for (int i = 0; i < MAX_KEY_TYPE_NUM; i++)
		{
			int keyConfigButton = enGameActionNum;
			// キータイプがスキルだったら
			if (i == SKILL_KEY_TYPE) {
				// 選択されているアクションがスキルじゃなかったら
				if (selectAction != enActionSpecialSkill) {
					keyConfigButton = KeyConfig::Get().GetBindButton(enActionSpecialSkill);
				}
			}
			if (i == RUN_KEY_TYPE) {
				if (selectAction != enActionDash) {
					keyConfigButton = KeyConfig::Get().GetBindButton(enActionDash);
				}
			}
			if (i == ATTACK_KEY_TYPE) {
				if (selectAction != enActionNormalSkill) {
					keyConfigButton = KeyConfig::Get().GetBindButton(enActionNormalSkill);
				}
			}

			if (keyConfigButton == inputButton) {
				return true;
			}
		}
		return false;
	}

	EnGameAction GetOverlapAction(const EnButton inputButton)
	{
		for (int i = 0; i < MAX_KEY_TYPE_NUM; i++)
		{
			// キータイプがスキルだったら
			if (i == SKILL_KEY_TYPE) {
				if (inputButton == KeyConfig::Get().GetBindButton(enActionSpecialSkill)) {
					return enActionSpecialSkill;
				}
			}
			// キータイプが回避だったら
			if (i == RUN_KEY_TYPE) {
				if (inputButton == KeyConfig::Get().GetBindButton(enActionDash)) {
					return enActionDash;
				}
			}
			// キータイプが攻撃だったら
			if (i == ATTACK_KEY_TYPE) {
				if (inputButton == KeyConfig::Get().GetBindButton(enActionNormalSkill)) {
					return enActionNormalSkill;
				}
			}
		}

		return enGameActionNum;
	}
}


bool KeyConfigOptionMenu::isWarningWindowCancel_ = false;

bool KeyConfigOptionMenu::isWarningWindowClose_ = false;


void KeyConfigOptionMenu::Update()
{
	taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	


	// デフォルト
	if (keyConfigState_ == KEY_CONFIG_STATE_SELECT) {
		if (g_pad[0]->IsTrigger(enButtonX)) {
			/*skillButton = DEFAULT_KEY_TYPE;
			runButton = DEFAULT_KEY_TYPE;
			attackButton = DEFAULT_KEY_TYPE;*/

			KeyConfig::Get().SetKeyBind(EnGameAction::enActionSpecialSkill, enButtonY);
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionDash, enButtonA);
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionNormalSkill, enButtonB);

			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);

			keyConfigState_ = KEY_CONFIG_STATE_DEFAULT;
		}
	}
	

	// @todo for test 一旦、分けてみる
	switch (keyConfigState_)
	{
		case KEY_CONFIG_STATE_INIT:
		{
			keyConfigState_ = KEY_CONFIG_STATE_SELECT;
		}
		break;

		case KEY_CONFIG_STATE_SELECT:
		{
			// 選択の青枠
			UpdateSelectFlame();

			if (g_pad[0]->IsTrigger(enButtonA))
			{
				// 青枠点滅：光
				auto* selectFlame = GetUI<UIIcon>(Hash32("flame"));
				auto* animation = selectFlame->FindAnimation(Hash32("KeyConfig_selectFlame_FadeIn"));
				animation->Clear();
				animation->Play();

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

				// 状態の切り替え
				keyConfigState_ = KEY_CONFIG_STATE_WAIT_ACTION;
			}
		}
		break;

		case KEY_CONFIG_STATE_WAIT_ACTION:
		{
			if (IsInputTargetKey())
			{
				// 次に設定するボタンと被った
				const auto selectAction = GetSelectAction();
				const auto selectButton = GetChangeButton(selectAction);
				const auto inputButton = GetInputButton();
				if (FindOverlapButton(inputButton, selectAction)) {
					// 今のアクションと設定されているボタン
					WarningButtonWindow::SetSelectAction(selectAction);
					WarningButtonWindow::SetSelectButton(selectButton);
					// 重なっているアクションと設定されているボタン
					const auto overlapAction = GetOverlapAction(inputButton);
					WarningButtonWindow::SetOverlapAction(overlapAction);
					WarningButtonWindow::SetOverlapButton(inputButton);

					selectAction_ = selectAction;
					selectButton_ = selectButton;

					overlapAction_ = overlapAction;
					overlapButton_ = inputButton;

					keyConfigState_ = KEY_CONFIG_STATE_OVERLAP;
					break;
				}

				// 選択されたボタンを設定
				UpdateSelectedButton(inputButton);

				// 選択されたボタンのみを画像切り替え
				{
					{
						switch (selectKeyConfigType)
						{
						case SKILL_KEY_TYPE:
							InitializeButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
							break;

						case RUN_KEY_TYPE:
							InitializeButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
							break;

						case ATTACK_KEY_TYPE:
							InitializeButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);
							break;
						}
					}
				}

				// 状態の切り替え
				keyConfigState_ = KEY_CONFIG_STATE_DECIDE;
			}
		}
		break;

		case KEY_CONFIG_STATE_DECIDE:
		{
			// 青枠点滅：消
			ResetSelectFlameAnimation();

			// 状態の切り替え
			keyConfigState_ = KEY_CONFIG_STATE_SELECT;

		}	
		break;

		case KEY_CONFIG_STATE_OVERLAP:
		{
			if (isWarningWindowCancel_) {
				keyConfigState_ = KEY_CONFIG_STATE_WAIT_ACTION;
				isWarningWindowCancel_ = false;
			}
			// 正常に閉じた
			if(isWarningWindowClose_){
				keyConfigState_ = KEY_CONFIG_STATE_OVERLAP_DECIDE;
				isWarningWindowClose_ = false;
			}
		}
		break;

		// 閉じたので2か所ボタン表示を切り替える
		case KEY_CONFIG_STATE_OVERLAP_DECIDE:
		{
			// 青枠点滅：消
			ResetSelectFlameAnimation();

			// 選択されたボタンを設定
			KeyConfig::Get().SetKeyBind(overlapAction_, selectButton_);
			KeyConfig::Get().SetKeyBind(selectAction_, overlapButton_);

			InitializeButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
			InitializeButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
			InitializeButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);

			keyConfigState_ = KEY_CONFIG_STATE_SELECT;
		}
		break;

		case KEY_CONFIG_STATE_DEFAULT:
		{
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionSpecialSkill, enButtonY);
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionDash, enButtonA);
			KeyConfig::Get().SetKeyBind(EnGameAction::enActionNormalSkill, enButtonB);

			{
				InitializeButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
				InitializeButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
				InitializeButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);
			}

			keyConfigState_ = KEY_CONFIG_STATE_SELECT;
		}
		break;
	}

	MenuBase::Update();
}


void KeyConfigOptionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void KeyConfigOptionMenu::InitializeLogic()
{
	// 選択状態を一番上(スキル)にリセット
	selectKeyConfigType = SKILL_KEY_TYPE;

	// タスクスケジューラーの生成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();	

	// 一回だけよぶアタッチ
	auto* selectFlame = GetUI<UIIcon>(Hash32("flame"));
	UIAnimationFactory::Attach<UIColorAnimation>(selectFlame, Hash32("KeyConfig_selectFlame_FadeIn"));

	const int id = taskScheduler_->CreateLoopSequence(20.0f);
	{
		nikukyuList_[0] = GetUI<UIIcon>(Hash32("optionBack/nikukyu1"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[0], Hash32("nikukyuu_fadein"));
		taskScheduler_->AddLoopTimer(id, 0.0f, [&]()
			{
				auto* animation = nikukyuList_[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler_->AddLoopTimer(id, 6.0f, [&]()
			{
				auto* animation = nikukyuList_[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList_[1] = GetUI<UIIcon>(Hash32("optionBack/nikukyu2"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[1], Hash32("nikukyuu_fadein"));
		taskScheduler_->AddLoopTimer(id, 4.0f, [&]()
			{
				auto* animation = nikukyuList_[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler_->AddLoopTimer(id, 10.0f, [&]()
			{
				auto* animation = nikukyuList_[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList_[2] = GetUI<UIIcon>(Hash32("optionBack/nikukyu3"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[2], Hash32("nikukyuu_fadein"));
		taskScheduler_->AddLoopTimer(id, 8.0f, [&]()
			{
				auto* animation = nikukyuList_[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler_->AddLoopTimer(id, 14.0f, [&]()
			{
				auto* animation = nikukyuList_[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList_[3] = GetUI<UIIcon>(Hash32("optionBack/nikukyu4"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[3], Hash32("nikukyuu_fadein"));
		taskScheduler_->AddLoopTimer(id, 12.0f, [&]()
			{
				auto* animation = nikukyuList_[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskScheduler_->AddLoopTimer(id, 18.0f, [&]()
			{
				auto* animation = nikukyuList_[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}

	{
		InitializeButtonIcon(Hash32("SkillButton/buttonA"), Hash32("SkillButton/buttonB"), Hash32("SkillButton/buttonX"), Hash32("SkillButton/buttonY"), enActionSpecialSkill);
		InitializeButtonIcon(Hash32("RunButton/buttonA"), Hash32("RunButton/buttonB"), Hash32("RunButton/buttonX"), Hash32("RunButton/buttonY"), enActionDash);
		InitializeButtonIcon(Hash32("AttackButton/buttonA"), Hash32("AttackButton/buttonB"), Hash32("AttackButton/buttonX"), Hash32("AttackButton/buttonY"), enActionNormalSkill);
	}
}

void KeyConfigOptionMenu::ResetSelectFlameAnimation()
{
	// 青枠点滅：消
	auto* selectFlame = GetUI<UIIcon>(Hash32("flame"));
	auto* dummyFlame = GetUI<UIDummy>(Hash32("InitFlame"));
	auto* animation = selectFlame->FindAnimation(Hash32("KeyConfig_selectFlame_FadeIn"));
	animation->Stop();
	selectFlame->color.w = dummyFlame->color.w;
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
	float offsetY = 225.0f;
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
}

bool KeyConfigOptionMenu::IsButtonOverLap() const
{
	// キーステートがボタン設定が被っている状態になっている
	return keyConfigState_ == KEY_CONFIG_STATE_OVERLAP;
}

bool KeyConfigOptionMenu::IsButtonSetting() const
{
	// ボタン設定中はBボタンを押しても設定画面に戻らない
	return keyConfigState_ == KEY_CONFIG_STATE_WAIT_ACTION;
}


void KeyConfigOptionMenu::InitializeButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int actionKey)
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

