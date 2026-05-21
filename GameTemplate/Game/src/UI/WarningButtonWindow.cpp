/**
 * WarningButtonWindow.cpp
 * 同じボタンが割り当てられている場合に表示する警告ウィンドウ
 */
#include "stdafx.h"
#include "WarningButtonWindow.h"
#include "UIAnimationFactory.h"
#include "Layout.h"

namespace
{
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

// 設定されているアクションとボタン
EnGameAction WarningButtonWindow::selectAction_ = enActionNormalSkill;
EnButton WarningButtonWindow::selectButton_ = enButtonNum;

// 重なっているアクションとボタン
EnGameAction WarningButtonWindow::overlapAction_ = enActionNormalSkill;
EnButton WarningButtonWindow::overlapButton_ = enButtonNum;


void WarningButtonWindow::Update()
{
	//KeyConfig::Get().GetBindButton();

	{
		UpdateButtonIcon(Hash32("AttackButton1/buttonA"), Hash32("AttackButton1/buttonB"), Hash32("AttackButton1/buttonX"), Hash32("AttackButton1/buttonY"), selectButton_, selectAction_);
		UpdateButtonIcon(Hash32("AttackButton2/buttonA"), Hash32("AttackButton2/buttonB"), Hash32("AttackButton2/buttonX"), Hash32("AttackButton2/buttonY"), overlapButton_, overlapAction_);
		UpdateButtonIcon(Hash32("AttackButton3/buttonA"), Hash32("AttackButton3/buttonB"), Hash32("AttackButton3/buttonX"), Hash32("AttackButton3/buttonY"), overlapButton_, overlapAction_);
		UpdateButtonIcon(Hash32("AttackButton4/buttonA"), Hash32("AttackButton4/buttonB"), Hash32("AttackButton4/buttonX"), Hash32("AttackButton4/buttonY"), selectButton_, selectAction_);
	}

	{
		UpdateButtonWord(Hash32("WordButton1/word_skill"), Hash32("WordButton1/word_run"), Hash32("WordButton1/word_attack"), selectAction_);
		UpdateButtonWord(Hash32("WordButton2/word_skill"), Hash32("WordButton2/word_run"), Hash32("WordButton2/word_attack"), overlapAction_);
		UpdateButtonWord(Hash32("WordButton3/word_skill"), Hash32("WordButton3/word_run"), Hash32("WordButton3/word_attack"), selectAction_);
		UpdateButtonWord(Hash32("WordButton4/word_skill"), Hash32("WordButton4/word_run"), Hash32("WordButton4/word_attack"), overlapAction_);
	}
	

	MenuBase::Update();
}


void WarningButtonWindow::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void WarningButtonWindow::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();
}



void WarningButtonWindow::UpdateSelectedButton(const int buttonType)
{
	if (g_pad[0]->IsTrigger(static_cast<EnButton>(buttonType))) {
			KeyConfig::Get().SetKeyBind(enActionSpecialSkill, buttonType);
			KeyConfig::Get().SetKeyBind(enActionDash, buttonType);
			KeyConfig::Get().SetKeyBind(enActionNormalSkill, buttonType);
	}
}


void WarningButtonWindow::UpdateButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int attackKey, const int actionKey)
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

void WarningButtonWindow::UpdateButtonWord(const uint32_t skillId, const uint32_t DashId, const uint32_t AttackId, const int actionKey)
{
	// 非表示
	auto* skillWord = GetUI<UIIcon>(skillId);
	skillWord->isDraw = false;
	auto* dashWord = GetUI<UIIcon>(DashId);
	dashWord->isDraw = false;
	auto* attackWord = GetUI<UIIcon>(AttackId);
	attackWord->isDraw = false;

	// 初期の変数
	//int actionKey = KeyConfig::Get().GetBindButton(static_cast<EnGameAction>(actionKey));
	// 指定された文字の表示
	if (actionKey == enActionSpecialSkill) {
		skillWord->isDraw = true;
	}
	if (actionKey == enActionDash) {
		dashWord->isDraw = true;
	}
	if (actionKey == enActionNormalSkill) {
		attackWord->isDraw = true;
	}
}
