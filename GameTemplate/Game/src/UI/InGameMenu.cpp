/**
 * InGameMenu.cpp
 * インゲームメニュー
 */
#include "stdafx.h"
#include "InGameMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h"

#include "src/Actor/Player.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/ActorStatus.h"

 


namespace
{
	//static TaskSchedulerSystem* taskScheduler = nullptr;

	static float playerHP = 10.0f;
	static float bossHP = 10.0f;

	/** 各アクションごとのボタンID */
	struct ActionTypeButtonId
	{
		uint32_t buttonAId;
		uint32_t buttonBId;
		uint32_t buttonXId;
		uint32_t buttonYId;
		//
		ActionTypeButtonId() {}
		ActionTypeButtonId(const uint32_t aId, const uint32_t bId, const uint32_t xId, const uint32_t yId)
			: buttonAId(aId), buttonBId(bId), buttonXId(xId), buttonYId(yId)
		{
		}
	};

	ActionTypeButtonId GetActionTypeButtonId(const int enAction)
	{
		switch (enAction)
		{
			case enActionSpecialSkill:
			{
				return ActionTypeButtonId(Hash32("AbilitySkillIcon/InGame_ButtonUI_Skill/inGame_ButtonIcon_A"), Hash32("AbilitySkillIcon/InGame_ButtonUI_Skill/inGame_ButtonIcon_B"), Hash32("AbilitySkillIcon/InGame_ButtonUI_Skill/inGame_ButtonIcon_X"), Hash32("AbilitySkillIcon/InGame_ButtonUI_Skill/inGame_ButtonIcon_Y"));
			}
			case enActionDash:
			{
				return ActionTypeButtonId(Hash32("InGame_ButtonUI_Run/inGame_ButtonIcon_A"), Hash32("InGame_ButtonUI_Run/inGame_ButtonIcon_B"), Hash32("InGame_ButtonUI_Run/inGame_ButtonIcon_X"), Hash32("InGame_ButtonUI_Run/inGame_ButtonIcon_Y"));
			}
			case enActionNormalSkill:
			{
				return ActionTypeButtonId(Hash32("InGame_ButtonUI_Attack/inGame_ButtonIcon_A"), Hash32("InGame_ButtonUI_Attack/inGame_ButtonIcon_B"), Hash32("InGame_ButtonUI_Attack/inGame_ButtonIcon_X"), Hash32("InGame_ButtonUI_Attack/inGame_ButtonIcon_Y"));
			}
		}
		K2_ASSERT(false, "ここに来るはずはない");
		return ActionTypeButtonId();
	}
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
	bool isTakeDamagePlayer = false;
	bool isTakeDamageBoss = false;
	int playerHP = 0;
	int playerMaxHP = 0;
	int bossHP = 0;
	int bossMaxHP = 0;
	auto* player = FindGO<Player>("player");
	auto* boss = FindGO<BossCharacter>("boss");

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

		isTakeDamagePlayer = playerStatus->IsTakeDamage();
	}

	if (boss){
		auto* bossStatus = boss->GetStatus()->As<BossStatus>();
		if (bossStatus) {
			bossHP = bossStatus->GetHP();
			bossMaxHP = bossStatus->GetMaxHP();

			isTakeDamageBoss = bossStatus->IsTakeDamage();
		}
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
	

	// ボスHPの増減
	auto* bossGauge = GetUI<UIIcon>(Hash32("BossHPBar/Boss_HP_gauge"));
	if (bossGauge)
	{
		bossGauge->transform.localScale.x = bossHP / static_cast<float>(bossMaxHP);
	}
	
	

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
				abilitySkillIconScaleSequence_->Play(abilitySkillIconCanvas);
			}
		}
		abilitySkillIconScaleSequence_->Update(g_gameTime->GetFrameDeltaTime());
		//// スキルボタンのキャンバス
		//auto* abilitySkillButtonIconCanvas = GetUI<UICanvas>(Hash32("InGame_ButtonUI_Skill"));
		//if (abilitySkillButtonIconCanvas) {
		//	if (isReadyAbilityFrame) {
		//		abilitySkillButtonIconScaleSequence_->Play(abilitySkillButtonIconCanvas);
		//	}
		//}
		//abilitySkillButtonIconScaleSequence_->Update(g_gameTime->GetFrameDeltaTime());
	}
	

	// プレイヤーがダメージを受けたとき
	{
		// 表情が変わる
		if (isTakeDamagePlayer)
		{
			CreatePlayerDamageScheduler();
		}
	}

	// ボスが被ダメをしたらHPバーが動く
	auto* bossHPCanvas = GetUI<UICanvas>(Hash32("BossHPBar"));
	if (bossHPCanvas) {
		if (isTakeDamageBoss)
		{
			bossHitHPPositionSequence_->Play(bossHPCanvas);
		}
	}
	bossHitHPPositionSequence_->Update(g_gameTime->GetFrameDeltaTime());
	
	if (playerDamageScheduler_) {
		playerDamageScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	}

	// 押されたボタンの表示
	{
		UpdateButtonWord(EnGameAction::enActionSpecialSkill);
		UpdateButtonWord(EnGameAction::enActionDash);
		UpdateButtonWord(EnGameAction::enActionNormalSkill);
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
	auto* bossHPGauge = GetUI<UIIcon>(Hash32("BossHPBar/Boss_HP_gauge"));
	bossHPGauge->SetPivot(Vector2(0.0f, 0.5f));

	// ボスの名前が変わる
	auto* bossGollira = GetUI<UIIcon>(Hash32("BossHPBar/Boss_HP_word"));
	auto* bossTurtle = GetUI<UIIcon>(Hash32("BossHPBar/Boss_HP_word2"));
	BossType stageType = CharacterDataBase::Get().GetStageType();
	// ゴリラが選択されているとき
	if (stageType == BossType::enGorilla) {
		bossGollira->isDraw = true;
	}
	// カメが選択されているとき
	if (stageType == BossType::enTurtle) {
		bossTurtle->isDraw = true;
	}

	// アビリティアイコンのキャンバス
	auto* abilitySkillIconCanvas = GetUI<UICanvas>(Hash32("AbilitySkillIcon"));
	UIAnimationFactory::Attach<UIScaleAnimation>(abilitySkillIconCanvas, Hash32("SkillReadyScaleUp"));
	UIAnimationFactory::Attach<UIScaleAnimation>(abilitySkillIconCanvas, Hash32("SkillReadyScaleDown"));
	abilitySkillIconScaleSequence_ = std::make_unique<UIAnimationSequence>();
	abilitySkillIconScaleSequence_->Add(Hash32("SkillReadyScaleUp")).Add(Hash32("SkillReadyScaleDown"));

	// ボスHPのキャンバス
	auto* bossHPCanvas = GetUI<UICanvas>(Hash32("BossHPBar"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossHPCanvas, Hash32("HitBossPositionUp"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossHPCanvas, Hash32("HitBossPositionDown"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossHPCanvas, Hash32("HitBossPositionUp2"));
	bossHitHPPositionSequence_ = std::make_unique<UIAnimationSequence>();
	bossHitHPPositionSequence_->Add(Hash32("HitBossPositionUp")).Add(Hash32("HitBossPositionDown"));
}


void InGameMenu::CreatePlayerDamageScheduler()
{
	// スケジューラーの作成
	playerDamageScheduler_ = std::make_unique<TaskSchedulerSystem>();
	// プレイヤー被ダメ時(スタート)
	playerDamageScheduler_->AddTimer(0.0f, [this]
		{
			// 表情が変わる
			auto* playerNormal = GetUI<UIIcon>(Hash32("Player_icon_normal"));
			// 普通のアイコンが消える
			playerNormal->isDraw = false;

			// ダメージ受けた時の顔に切り替え
			auto* playerDamage = GetUI<UIIcon>(Hash32("Player_icon_damage"));
			playerDamage->isDraw = true;

			// ダメージ受けたら枠の色が変化
			auto* normalFlame = GetUI<UIIcon>(Hash32("Player_icon_flame"));
			auto* damageColorDummy = GetUI<UIDummy>(Hash32("FlameDamageColorDummy"));
			normalFlame->color = damageColorDummy->color;
		});
	// プレイヤー被ダメ時(おわり)
	playerDamageScheduler_->AddTimer(0.8f, [this]
		{
			// 表情が変わる
			auto* playerNormal = GetUI<UIIcon>(Hash32("Player_icon_normal"));
			// 普通のアイコンが消える
			// ダメージ受けた時のアイコンが消える
			playerNormal->isDraw = true;

			// 普通のアイコンに切り替わる
			auto* playerDamage = GetUI<UIIcon>(Hash32("Player_icon_damage"));
			playerDamage->isDraw = false;

			// 枠の色を元に戻す
			auto* normalFlame = GetUI<UIIcon>(Hash32("Player_icon_flame"));
			auto* normalColorDummy = GetUI<UIDummy>(Hash32("FlameNormalColorDummy"));
			normalFlame->color = normalColorDummy->color;
		});
}

void InGameMenu::UpdateButtonWord(const uint32_t enAction)
{
	const auto actionTypeButtonId = GetActionTypeButtonId(enAction);

	// 非表示
	auto* buttonA = GetUI<UIIcon>(actionTypeButtonId.buttonAId);
	buttonA->isDraw = false;
	auto* buttonB = GetUI<UIIcon>(actionTypeButtonId.buttonBId);
	buttonB->isDraw = false;
	auto* buttonX = GetUI<UIIcon>(actionTypeButtonId.buttonXId);
	buttonX->isDraw = false;
	auto* buttonY = GetUI<UIIcon>(actionTypeButtonId.buttonYId);
	buttonY->isDraw = false;


	// ボタン設定されたときにそのボタンをとってくる
	const int key = KeyConfig::Get().GetBindButton(static_cast<EnGameAction>(enAction));

	// 特定のボタンが押されたら表示
	if (key == enButtonA) {
		buttonA->isDraw = true;
	}
	if (key == enButtonB) {
		buttonB->isDraw = true;
	}
	if (key == enButtonX) {
		buttonX->isDraw = true;
	}
	if (key == enButtonY) {
		buttonY->isDraw = true;
	}
}
