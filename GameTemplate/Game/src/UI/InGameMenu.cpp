/**
 * InGameMenu.cpp
 * サウンドオプションメニュー
 */
#include "stdafx.h"
#include "InGameMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h"


namespace
{
	//static TaskSchedulerSystem* taskScheduler = nullptr;

	static float playerHP = 10.0f;
	static float bossHP = 10.0f;

	static Vector4 attack_button = { 1.0f, 1.0f, 1.0f, 1.0f };
	static Vector4 dodeg_button = { 1.0f, 1.0f, 1.0f, 1.0f };
}


void InGameMenu::Update()
{
	//taskScheduler->Update(g_gameTime->GetFrameDeltaTime());

	// プレイヤーHPの増減
	auto* playerGauge = GetUI<UIIcon>(Hash32("Player_HP_gauge"));
	if(playerGauge)
	{
		if (g_pad[0]->IsTrigger(enButtonDown)) {
			playerHP -= 1.0f;
			if (playerHP < 0.0f) {
				playerHP = 0.0f;
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonUp)) {
			playerHP += 1.0f;
			if (playerHP > 10.0f){
				playerHP = 10.0f;
			}
		}
	}
	playerGauge->transform.localScale.x = playerHP / 10.0f;
	

	// プレイヤーHPの増減
	auto* bossGauge = GetUI<UIIcon>(Hash32("Boss_HP_gauge"));
	if (bossGauge)
	{
		if (g_pad[0]->IsTrigger(enButtonDown)) {
			bossHP -= 1.0f;
			if (bossHP < 0.0f) {
				bossHP = 0.0f;
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonUp)) {
			bossHP += 1.0f;
			if (bossHP > 10.0f) {
				bossHP = 10.0f;
			}
		}
	}
	bossGauge->transform.localScale.x = bossHP / 10.0f;
	

	// 攻撃ボタン枠の色変化
	auto* attackButton = GetUI<UIIcon>(Hash32("Attack_Icon_flame")); 
	if(attackButton){
		if (g_pad[0]->IsPress(enButtonRight)) {
			attack_button = Vector4{ 0.92f,0.57f,0.31f,1.0f };
		}
		else if(!g_pad[0]->IsPress(enButtonRight)){
			attack_button = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
		}
	}
	attackButton->color = attack_button;
	
	// 回避ボタン枠の色変化
	auto* dodegButton = GetUI<UIIcon>(Hash32("dodeg_Icon_flame"));
	if (dodegButton) {
		if (g_pad[0]->IsPress(enButtonLeft)) {
			dodeg_button = Vector4{ 0.92f, 0.57f, 0.31f, 1.0f };
		}
		else if (!g_pad[0]->IsPress(enButtonLeft)) {
			dodeg_button = Vector4{ 1.0f, 1.0f, 1.0f, 1.0f };
		}
	}
	dodegButton->color = dodeg_button;


	auto* dummy = GetUI<UIDummy>(Hash32("AbilitySkillIcon/ColorDummy"));
	auto* skillIconFrame = GetUI<UIIcon>(Hash32("AbilitySkillIcon/SkillIconFlame"));
	if (skillIconFrame) {
		bool isChange = g_pad[0]->IsPress(enButtonLeft);
		skillIconFrame->color = isChange ? dummy->color : Vector4::White;
	}

	MenuBase::Update();
}


void InGameMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void InGameMenu::InitializeLogic()
{
	auto* playerHPGauge = GetUI<UIIcon>(Hash32("Player_HP_gauge"));
	playerHPGauge->SetPivot(Vector2(0.0f, 0.5f));
	auto* bossHPGauge = GetUI<UIIcon>(Hash32("Boss_HP_gauge"));
	bossHPGauge->SetPivot(Vector2(0.0f, 0.5f));
}