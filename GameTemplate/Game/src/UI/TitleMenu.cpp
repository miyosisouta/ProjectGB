/**
 * InGameMenu.cpp
 * タイトルメニュー
 */
#include "stdafx.h"
#include "TitleMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h" 


namespace
{
	IntSelector selector(0, 2, 1, 0);
}


void TitleMenu::Update()
{
	selector.Update();
	

	auto* startPositionDummy = GetUI<UIDummy>(Hash32("StartPositionDummy"));
	auto* soundPositionDummy = GetUI<UIDummy>(Hash32("SoundPositionDummy"));
	auto* exitPositionDummy = GetUI<UIDummy>(Hash32("ExitPositionDummy"));
	
	auto* selectIcon = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
	if (selectIcon) {
		const int selectIndex = selector.GetValue();
		Vector3 selectPositionList[] = {
			startPositionDummy->transform.position,
			soundPositionDummy->transform.position,
			exitPositionDummy->transform.position
		};
		selectIcon->transform.localPosition = selectPositionList[selectIndex];
	}


	// @todo for test
	if (g_pad[0]->IsTrigger(enButtonB))
	{
		// Aボタンの透明度
		auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
		AbuttonColor->isDraw = false;
		
		// 非表示 はじめる
		auto* start = GetUI<UIIcon>(Hash32("Title_start"));
		start->isDraw = true;

		// 非表示 サウンド
		auto* sound = GetUI<UIIcon>(Hash32("Title_sound"));
		sound->isDraw = true;

		// 非表示 おわる
		auto* exit = GetUI<UIIcon>(Hash32("Title_exit"));
		exit->isDraw = true;

		// 非表示 にくきゅう
		auto* select = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
		select->isDraw = true;
	}

	
	//// プレイヤーHPの増減
	//auto* playerGauge = GetUI<UIIcon>(Hash32("Player_HP_gauge"));
	//if(playerGauge)
	//{
	//	playerGauge->transform.localScale.x = playerHP / static_cast<float>(playerMaxHP);
	//}
	//

	//// プレイヤーHPの増減
	//auto* bossGauge = GetUI<UIIcon>(Hash32("Boss_HP_gauge"));
	//if (bossGauge)
	//{
	//	if (g_pad[0]->IsTrigger(enButtonDown)) {
	//		bossHP -= 1.0f;
	//		if (bossHP < 0.0f) {
	//			bossHP = 0.0f;
	//		}
	//	}
	//	else if (g_pad[0]->IsTrigger(enButtonUp)) {
	//		bossHP += 1.0f;
	//		if (bossHP > 10.0f) {
	//			bossHP = 10.0f;
	//		}
	//	}
	//}
	//bossGauge->transform.localScale.x = bossHP / 10.0f;
	//

	//// 攻撃ボタン枠の色変化
	//auto* attackButton = GetUI<UIIcon>(Hash32("Attack_Icon_flame")); 
	//if(attackButton){
	//	attackButton->color = isUseNormalAttack ? frameIconColor : Vector4::White;
	//}
	//
	//// 回避ボタン枠の色変化
	//auto* dodegButton = GetUI<UIIcon>(Hash32("dodeg_Icon_flame"));
	//if (dodegButton) {
	//	dodegButton->color = isUseDodgeRoll ? frameIconColor : Vector4::White;
	//}

	//// スキルボタン
	//{
	//	auto* dummyColor = GetUI<UIDummy>(Hash32("AbilitySkillIcon/CoolDownColorDummy"));
	//	const Vector4 coolDownColor = dummyColor->color;

	//	// スキルボタン枠の色変化
	//	auto* skillIconFrame = GetUI<UIIcon>(Hash32("AbilitySkillIcon/SkillIconFlame"));
	//	if (skillIconFrame) {
	//		Vector4 color = Vector4::White;
	//		if (isUseAbility) {
	//			color = frameIconColor;
	//		}
	//		if (isCoolDownAbility) {
	//			color = coolDownColor;
	//		}
	//		skillIconFrame->color = color;
	//	}
	//	// スキルボタンの色変化
	//	auto* skillIconBack = GetUI<UIIcon>(Hash32("AbilitySkillIcon/skillIconBack"));
	//	if (skillIconBack) {
	//		// クールダウン中なら暗くしたい
	//		// isCoolDownAbility == true なら、白で、それ以外は暗くする色をダミーからとってくる
	//		skillIconBack->color = isCoolDownAbility ? coolDownColor : Vector4::White;
	//	}
	//	// コントローラーボタンアイコン
	//	auto* buttonIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/buttonIcon"));
	//	if (buttonIcon) {
	//		buttonIcon->color = isCoolDownAbility ? coolDownColor : Vector4::White;
	//	}
	//	// コントローラーボタン背景アイコン
	//	auto* buttonBackIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/ButtonIconback"));
	//	if (buttonBackIcon) {
	//		buttonBackIcon->color = isCoolDownAbility ? coolDownColor : Vector4::White;
	//	}
	//	// スキルボタンのキャンバス
	//	auto* abilitySkillIconCanvas = GetUI<UICanvas>(Hash32("AbilitySkillIcon"));
	//	if (abilitySkillIconCanvas) {
	//		if (isReadyAbilityFrame) {
	//			abilitySkillIconScaleSequence->Play(abilitySkillIconCanvas);
	//		}
	//	}
	//	abilitySkillIconScaleSequence->Update(g_gameTime->GetFrameDeltaTime());
	//}
	


	MenuBase::Update();
}


void TitleMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void TitleMenu::InitializeLogic()
{
	// Aボタンの透明度
	auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
	UIAnimationFactory::Attach<UIColorAnimation>(AbuttonColor,Hash32("pushAbutton_fadeout"));
	{
		auto* animation = AbuttonColor->FindAnimation(Hash32("pushAbutton_fadeout"));
		animation->Play();
	}

	// 非表示 はじめる
	auto* start = GetUI<UIIcon>(Hash32("Title_start"));
	start->isDraw = false;

	// 非表示 サウンド
	auto* sound = GetUI<UIIcon>(Hash32("Title_sound"));
	sound->isDraw = false;

	// 非表示 おわる
	auto* exit = GetUI<UIIcon>(Hash32("Title_exit"));
	exit->isDraw = false;

	// 非表示 にくきゅう
	auto* select = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
	select->isDraw = false;
}