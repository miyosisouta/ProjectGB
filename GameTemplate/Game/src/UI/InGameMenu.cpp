/**
 * InGameMenu.cpp
 * サウンドオプションメニュー
 */
#include "stdafx.h"
#include "InGameMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h"

#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"

 


namespace
{
	//static TaskSchedulerSystem* taskScheduler = nullptr;

	static float playerHP = 10.0f;
	static float bossHP = 10.0f;
}


void InGameMenu::Update()
{
	// NOTE: FindoGOにて行う
	// 　　　できれば管理者から受け取るようにしたい
	bool isUseNormalAttack = false;
	bool isUseAbility = false;
	bool isUseDodgeRoll = false;
	bool isCoolDownAbility = false;
	bool isReadyAbilityFrame = false;
	bool isTakeDamage = false;
	int playerHP = 0;
	int playerMaxHP = 0;
	auto* player = FindGO<Player>("player");
	if (player) {
		auto* stateMachine = player->GetStateMachine();
		isUseNormalAttack = stateMachine->IsActionButtonB();
		isUseAbility = stateMachine->IsActionButtonY();
		isUseDodgeRoll = stateMachine->IsActionButtonX();

		auto* playerStatus = player->GetStatus()->As<PlayerStatus>();
		isCoolDownAbility = !playerStatus->CanExecuteSpecialAbility();
		isReadyAbilityFrame = playerStatus->IsReadyFrameSpecialAbility();

		playerHP = playerStatus->GetHP();
		playerMaxHP = playerStatus->GetMaxHP();

		isTakeDamage = playerStatus->IsTakeDamage();
	}

	Vector4 frameIconColor = Vector4::White;
	auto* dummy = GetUI<UIDummy>(Hash32("FrameColorDummy"));
	if (dummy) {
		frameIconColor = dummy->color;
	}

	// プレイヤーHPの増減
	auto* playerGauge = GetUI<UIIcon>(Hash32("Player_HP_gauge"));
	if(playerGauge)
	{
		playerGauge->transform.localScale.x = playerHP / static_cast<float>(playerMaxHP);
	}
	

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
		attackButton->color = isUseNormalAttack ? frameIconColor : Vector4::White;
	}
	
	// 回避ボタン枠の色変化
	auto* dodegButton = GetUI<UIIcon>(Hash32("dodeg_Icon_flame"));
	if (dodegButton) {
		dodegButton->color = isUseDodgeRoll ? frameIconColor : Vector4::White;
	}

	// スキルボタン
	{
		auto* dummyColor = GetUI<UIDummy>(Hash32("AbilitySkillIcon/CoolDownColorDummy"));
		const Vector4 coolDownColor = dummyColor->color;

		// スキルボタン枠の色変化
		auto* skillIconFrame = GetUI<UIIcon>(Hash32("AbilitySkillIcon/SkillIconFlame"));
		if (skillIconFrame) {
			Vector4 color = Vector4::White;
			if (isUseAbility) {
				color = frameIconColor;
			}
			if (isCoolDownAbility) {
				color = coolDownColor;
			}
			skillIconFrame->color = color;
		}
		// スキルボタンの色変化
		auto* skillIconBack = GetUI<UIIcon>(Hash32("AbilitySkillIcon/skillIconBack"));
		if (skillIconBack) {
			// クールダウン中なら暗くしたい
			// isCoolDownAbility == true なら、白で、それ以外は暗くする色をダミーからとってくる
			skillIconBack->color = isCoolDownAbility ? coolDownColor : Vector4::White;
		}
		// コントローラーボタンアイコン
		auto* buttonIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/buttonIcon"));
		if (buttonIcon) {
			buttonIcon->color = isCoolDownAbility ? coolDownColor : Vector4::White;
		}
		// コントローラーボタン背景アイコン
		auto* buttonBackIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/ButtonIconback"));
		if (buttonBackIcon) {
			buttonBackIcon->color = isCoolDownAbility ? coolDownColor : Vector4::White;
		}
		// スキルボタンのキャンバス
		auto* abilitySkillIconCanvas = GetUI<UICanvas>(Hash32("AbilitySkillIcon"));
		if (abilitySkillIconCanvas) {
			if (isReadyAbilityFrame) {
				abilitySkillIconScaleSequence->Play(abilitySkillIconCanvas);
			}
		}
		abilitySkillIconScaleSequence->Update(g_gameTime->GetFrameDeltaTime());
	}
	

	// プレイヤーがダメージを受けたとき
	{
		// 表情が変わる
		auto* playerNormal = GetUI<UIIcon>(Hash32("Player_icon_normal"));
		if (playerNormal)
		{
			if (isTakeDamage)
			{
				// 普通のアイコンが消える
				playerNormal->isDraw = false;

				// ダメージ受けた時の顔に切り替え
				auto* playerDamage = GetUI<UIIcon>(Hash32("Player_icon_damage"));
				playerDamage->isDraw = true;
			}
			else
			{
				// ダメージ受けた時のアイコンが消える
				playerNormal->isDraw = true;

				// 普通のアイコンに切り替わる
				auto* playerDamage = GetUI<UIIcon>(Hash32("Player_icon_damage"));
				playerDamage->isDraw = false;
			}
		}

		// 枠が変わる
		auto* normalFlame = GetUI<UIIcon>(Hash32("Player_icon_flame"));
		if (normalFlame)
		{
			auto* damageColorDummy = GetUI<UIDummy>(Hash32("FlameDamageColorDummy"));
			auto* normalColorDummy = GetUI<UIDummy>(Hash32("FlameNormalColorDummy"));
			if (isTakeDamage)
			{
				normalFlame->color = damageColorDummy->color;
			}
			else
			{
				normalFlame->color = normalColorDummy->color;
			}
		}
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


	// アビリティアイコンのキャンバス
	auto* abilitySkillIconCanvas = GetUI<UICanvas>(Hash32("AbilitySkillIcon"));
	UIAnimationFactory::Attach<UIScaleAnimation>(abilitySkillIconCanvas, Hash32("SkillReadyScaleUp"));
	UIAnimationFactory::Attach<UIScaleAnimation>(abilitySkillIconCanvas, Hash32("SkillReadyScaleDown"));
	abilitySkillIconScaleSequence = std::make_unique<UIAnimationSequence>();
	abilitySkillIconScaleSequence->Add(Hash32("SkillReadyScaleUp")).Add(Hash32("SkillReadyScaleDown"));
}