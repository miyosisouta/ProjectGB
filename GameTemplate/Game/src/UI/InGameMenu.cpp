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
	float stamina = 0.0f;
	float maxStamina = 1.0f;
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

		stamina = playerStatus->GetStamina();
		maxStamina = playerStatus->GetMaxStamina();

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
	}
	

	// プレイヤーがダメージを受けたとき
	{
		// 表情が変わる
		if (isTakeDamagePlayer)
		{
			CreatePlayerDamageScheduler();
		}
	}

	//ボスのダメージ表示
	// 一定時間内の連続表示を抑制。ダメージ表示の重複を防ぐ
	if (damageNotifyCoolDown_ > 0.0f) {
		damageNotifyCoolDown_ -= g_gameTime->GetFrameDeltaTime();
	}
	// ダメージ通知を取得
	DamageNotify notify = BattleManager::Get().PopDamageNotify();
	// プレイヤーの攻撃が当たったときのみダメージ表示
	if (notify.damage > 0 && notify.type == DamageNotifyType::AttackHit)
	{
		// クールダウン終了じにダメージ表示
		if (damageNotifyCoolDown_ <= 0.0f)
		{
			UpdateBossHitDamage(notify);
			// 次の表示まで待機
			damageNotifyCoolDown_ = damageNotifyCoolDownTime;
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
	// ダメージ＆クリティカル表示のアニメーション
	for(int i = 0; i < DAMAGE_POOL_SIZE; i++)
	{
		if (bossHitDamageScheduler_[i]) {
			bossHitDamageScheduler_[i]->Update(g_gameTime->GetFrameDeltaTime());
		}
		if (bossCriticalHitDamageScheduler_[i]) {
			bossCriticalHitDamageScheduler_[i]->Update(g_gameTime->GetFrameDeltaTime());
		}
	}
	
	if (playerDamageScheduler_) {
		playerDamageScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	}

	// 押されたボタンの表示
	{
		UpdateButtonWord(EnGameAction::enActionSpecialSkill);
		UpdateButtonWord(EnGameAction::enActionDash);
		UpdateButtonWord(EnGameAction::enActionNormalSkill);
	}

	// スタミナの計算
	float staminaRate = stamina / maxStamina;
	
	staminaGauge_.SetProgressRange(0.0, staminaRate);
	staminaGauge_.Update();

	// スタミナの色を変える
	if (staminaRate >= 1.0f)
	{
		staminaGauge_.SetGaugeColor(Vector4(0.0f, 0.0f, 0.0f, 0.0f));
		
	}
	else if (staminaRate >= 0.5f)
	{
		staminaGauge_.SetGaugeColor({ 0.3f,1.0f,0.3f,1.0f });
		
	}
	else if (staminaRate >= 0.25f)
	{
		staminaGauge_.SetGaugeColor({ 1.0f,1.0f,0.3f,1.0f });
	}
	else
	{
		staminaGauge_.SetGaugeColor({ 1.0f,0.3f,0.3f,1.0f });
	}


	MenuBase::Update();
}


void InGameMenu::Render(RenderContext& rc)
{
	staminaGauge_.Draw(rc);
	MenuBase::Render(rc);
}


void InGameMenu::InitializeLogic()
{
	// スケジューラーの初期化 
	for (int i = 0; i < DAMAGE_POOL_SIZE; i++)
	{
		bossHitDamageScheduler_[i] = std::make_unique<TaskSchedulerSystem>();
		bossCriticalHitDamageScheduler_[i] = std::make_unique<TaskSchedulerSystem>();
	}
	
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

	// 選択されたスキルのアイコンを表示
	auto* biteIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/SkillIcon_Howl"));
	auto* landmineIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/SkillIcon_landmine"));
	auto* fireIcon = GetUI<UIIcon>(Hash32("AbilitySkillIcon/SkillIcon_fire"));
	AbilityType skillType = CharacterDataBase::Get().GetPlayerParam().ability;
	// かみつきが選択されているとき
	if (skillType == AbilityType::enDefault) {
		biteIcon->isDraw = true;
	}
	// かみつきが選択されているとき
	if (skillType == AbilityType::enLandmine) {
		landmineIcon->isDraw = true;
	}
	// かみつきが選択されているとき
	if (skillType == AbilityType::enFireMagic) {
		fireIcon->isDraw = true;
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

	// スタミナ
	staminaGauge_.Init(150.0, 150.0);
	staminaGauge_.SetPosition({80.0f,50.0f,0.0f});
	staminaGauge_.SetScale({ 0.5f, 0.5f, 1.0f });
	staminaGauge_.SetThickness(0.2f, 0.12f);
	staminaGauge_.SetProgressRangeDeg(0, 360);
	staminaGauge_.SetGaugeColor({ 0.3f,1.0f,0.3f,1.0f });
	//staminaGauge_.SetBgColor({ 0.3f,0.3f,0.3f,0.0f });
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

void InGameMenu::BossHitAnimationScheduler(int poolIndex)
{
	// poolIndexに対応したダメージ数字UIを取得
	const uint32_t damageNumAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/damageNumber0"),
		Hash32("InGame_Boss_HitDamage/damageNumber1"),
		Hash32("InGame_Boss_HitDamage/damageNumber2"),
	};
	const uint32_t currentHash = damageNumAssets[poolIndex];

	// ダメージ表示用スケジューラーの取得
	auto& scheduler = bossHitDamageScheduler_[poolIndex];
	// スケジューラーリセット
	bossHitDamageScheduler_[poolIndex]->Reset();
	// ヒットダメージの表示
	bossHitDamageScheduler_[poolIndex]->AddTimer(0.0f, [this, currentHash]
		{
			// アニメーション
			auto* bossDamage = GetUI<UIDigit>(currentHash);
			bossDamage->isDraw = true;
			K2_LOG("Scheduler isDraw = true, color.w = %f", bossDamage->color.w);
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossDamage, Hash32("damageDisplay_posUp1"));
			auto* animation = bossDamage->FindAnimation(Hash32("damageDisplay_posUp1"));
			K2_LOG("bossDamage=%p", bossDamage);
			K2_LOG("anim=%p", animation);
			if (animation) {
				animation->Clear();
				animation->Play();
			}
		});
	bossHitDamageScheduler_[poolIndex]->AddTimer(0.2f, [this, currentHash]
		{
			auto* bossDamage = GetUI<UIDigit>(currentHash);
			auto* animation = bossDamage->FindAnimation(Hash32("damageDisplay_posUp1"));
			if (animation) {
				animation->Stop();
			}
		});
	bossHitDamageScheduler_[poolIndex]->AddTimer(0.2f, [this, currentHash]
		{
			// アニメーション
			auto* bossDamage = GetUI<UIDigit>(currentHash);
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossDamage, Hash32("damageDisplay_posDown"));
			auto* animation = bossDamage->FindAnimation(Hash32("damageDisplay_posDown"));
			if (animation) {
				animation->Clear();
				animation->Play();
			}
		});
	bossHitDamageScheduler_[poolIndex]->AddTimer(0.4f, [this, currentHash]
		{
			auto* bossDamage = GetUI<UIDigit>(currentHash);
			auto* animation = bossDamage->FindAnimation(Hash32("damageDisplay_posDown"));
			if (animation) {
				animation->Stop();
			}
		});
	bossHitDamageScheduler_[poolIndex]->AddTimer(0.7f, [this, currentHash]
		{
			// アニメーション
			auto* bossDamage = GetUI<UIDigit>(currentHash);
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossDamage, Hash32("damageDisplay_posUp2"));
			UIAnimationFactory::Attach<UIColorAnimation>(bossDamage, Hash32("damageDisplay_fade"));
			auto* animation = bossDamage->FindAnimation(Hash32("damageDisplay_posUp2"));
			auto* fadeAnim = bossDamage->FindAnimation(Hash32("damageDisplay_fade"));
			if (animation  && fadeAnim) {
				animation->Clear();
				fadeAnim->Clear();
				animation->Play();
				fadeAnim->Play();
			}
		});
	bossHitDamageScheduler_[poolIndex]->AddTimer(1.1f, [this, currentHash]
		{
			auto* bossDamage = GetUI<UIDigit>(currentHash);
			auto* animation = bossDamage->FindAnimation(Hash32("damageDisplay_posUp2"));
			auto* fadeAnim = bossDamage->FindAnimation(Hash32("damageDisplay_fade"));
			if (animation && fadeAnim) {
				animation->Stop();
				fadeAnim->Stop();
				bossDamage->isDraw = false;
			}
		});
}

void InGameMenu::BossCriticalHitAnimationScheduler(int poolIndex)
{
	// poolIndexに対応したクリティカルUIを取得
	const uint32_t ciriticalAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/criticalHit0"),
		Hash32("InGame_Boss_HitDamage/criticalHit1"),
		Hash32("InGame_Boss_HitDamage/criticalHit2"),
	};
	const uint32_t currentHash = ciriticalAssets[poolIndex];
	// リセット
	bossCriticalHitDamageScheduler_[poolIndex]->Reset();
	// ボスのクリティカル被ダメ
	bossCriticalHitDamageScheduler_[poolIndex]->AddTimer(0.05f, [this, currentHash]
		{
			// アニメーション
			auto* bossCriticalDamage = GetUI<UIIcon>(currentHash);
			auto* bossCriticalDamageScale = GetUI<UIIcon>(currentHash);
			bossCriticalDamage->isDraw = true;
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossCriticalDamage, Hash32("damageDisplay_posUp1"));
			UIAnimationFactory::Attach<UIScaleAnimation>(bossCriticalDamageScale, Hash32("damageDisplay_scaleUp"));
			auto* animation = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_posUp1"));
			auto* scaleAnim = bossCriticalDamageScale->FindAnimation(Hash32("damageDisplay_scaleUp"));
			if (animation && scaleAnim) {
				animation->Clear();
				scaleAnim->Clear();
				animation->Play();
				scaleAnim->Play();
			}
		});
	bossCriticalHitDamageScheduler_[poolIndex]->AddTimer(0.25f, [this, currentHash]
		{
			auto* bossCriticalDamage = GetUI<UIIcon>(currentHash);
			auto* bossCriticalDamageScale = GetUI<UIIcon>(currentHash);
			auto* animation = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_posUp1"));
			auto* scaleAnim = bossCriticalDamageScale->FindAnimation(Hash32("damageDisplay_scaleUp"));
			if (animation && scaleAnim) {
				animation->Stop();
				scaleAnim->Stop();
			}
		});
	bossCriticalHitDamageScheduler_[poolIndex]->AddTimer(0.25f, [this, currentHash]
		{
			// アニメーション
			auto* bossCriticalDamage = GetUI<UIIcon>(currentHash);
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossCriticalDamage, Hash32("damageDisplay_posDown"));
			auto* animation = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_posDown"));
			if (animation) {
				animation->Clear();
				animation->Play();
			}
		});
	bossCriticalHitDamageScheduler_[poolIndex]->AddTimer(0.45f, [this, currentHash]
		{
			auto* bossCriticalDamage = GetUI<UIIcon>(currentHash);
			auto* animation = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_posDown"));
			if (animation) {
				animation->Stop();
			}
		});
	bossCriticalHitDamageScheduler_[poolIndex]->AddTimer(0.7f, [this, currentHash]
		{
			// アニメーション
			auto* bossCriticalDamage = GetUI<UIIcon>(currentHash);
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(bossCriticalDamage, Hash32("damageDisplay_posUp2"));
			UIAnimationFactory::Attach<UIColorAnimation>(bossCriticalDamage, Hash32("damageDisplay_fade"));
			auto* animation = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_posUp2"));
			auto* fadeAnim = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_fade"));
			if (animation && fadeAnim) {
				animation->Clear();
				fadeAnim->Clear();
				animation->Play();
				fadeAnim->Play();
			}
		});
	bossCriticalHitDamageScheduler_[poolIndex]->AddTimer(1.1f, [this, currentHash]
		{
			auto* bossCriticalDamage = GetUI<UIIcon>(currentHash);
			auto* animation = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_posUp2"));
			auto* fadeAnim = bossCriticalDamage->FindAnimation(Hash32("damageDisplay_fade"));
			if (animation && fadeAnim) {
				animation->Stop();
				fadeAnim->Stop();
				bossCriticalDamage->isDraw = false;
			}
		});
}

void InGameMenu::UpdateBossHitDamage(const DamageNotify& notify)
{
	// ボスの取得
	auto* boss = FindGO<BossCharacter>("boss");
	if (!boss) { return; }

	//ダメージ表示用UI
	const uint32_t damageNumAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/damageNumber0"),
		Hash32("InGame_Boss_HitDamage/damageNumber1"),
		Hash32("InGame_Boss_HitDamage/damageNumber2"),
	};

	// 今回使うダメージ表示UI
	const int currentIndex = damagePoolIndex_;

	// ダメージ数字UIの取得
	auto* damageNum = GetUI<UIDigit>(damageNumAssets[currentIndex]);
	if (!damageNum) { return; }

	// 次に使用するダメージ表示UIの決定
	damagePoolIndex_ = (damagePoolIndex_ + 1) % DAMAGE_POOL_SIZE;

	// ボスのワールド3D座標を取得
	Vector3 worldPos = boss->GetTransformPosition();
	// ダメージ表示の位置を計算するためのスクリーン座標
	Vector2 screenPos;
	// ボスの3D空間座標→2D画面上の座標変換
	g_camera3D->CalcScreenPositionFromWorldPosition(screenPos, worldPos);
	
	// NDC座標→スクリーン座標への変換（CalcScreenPositionFromWorldPositionの不足分を補う）
	float half_w = (float)g_graphicsEngine->GetFrameBufferWidth() * 0.5f;
	float half_h = (float)g_graphicsEngine->GetFrameBufferHeight() * 0.5f;
	screenPos.x = screenPos.x + half_w;
	screenPos.y = -screenPos.y + half_h;  // Y軸反転

	// ダメージ表示の位置をボスの位置へ
	auto* canvas = GetUI<UICanvas>(Hash32("InGame_Boss_HitDamage"));
	if (canvas)
	{
		// 最新のダメージ表示が最前面にくる
		canvas->MoveToFront(damageNumAssets[currentIndex]);
		// ダメージ表示をボスの画面上の位置に合わせる
		canvas->transform.localPosition.x = screenPos.x - half_w;
		canvas->transform.localPosition.y = screenPos.y - half_h;
	}

	// UIアニメーションの初期化
	BossHitAnimationResetIcon(currentIndex);
	// ダメージ数値の設定
	damageNum->SetNumber(notify.damage);
	// 通常ダメージのアニメーション
	BossHitAnimationScheduler(currentIndex);
	// クリティカルダメージのアニメーション
	if (notify.isCritical)
	{
		BossCriticalHitAnimationScheduler(currentIndex);
	}
}

void InGameMenu::BossHitAnimationResetIcon(int poolIndex)
{
	// ダメージ数字UI
	const uint32_t damageNumAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/damageNumber0"),
		Hash32("InGame_Boss_HitDamage/damageNumber1"),
		Hash32("InGame_Boss_HitDamage/damageNumber2"),
	};
	// ダメージ数字UI：オフセット用
	const uint32_t damageOffsetAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/DamageNumOffset0"),
		Hash32("InGame_Boss_HitDamage/DamageNumOffset1"),
		Hash32("InGame_Boss_HitDamage/DamageNumOffset2"),
	};

	// ボスの被ダメ
	auto* damageNumber = GetUI<UIDigit>(damageNumAssets[poolIndex]);
	auto* damageNumberInit = GetUI<UIDummy>(Hash32("InGame_Boss_HitDamage/InitDamageNumber"));
	auto* damageOffsetDummy = GetUI<UIDummy>(damageOffsetAssets[poolIndex]);
	// アニメーション停止
	if (auto* animation = GetUI<UIDigit>(damageNumAssets[poolIndex]))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	damageNumber->transform.localPosition = damageNumberInit->transform.localPosition;
	// .jsonオフセット
	if (damageOffsetDummy) 
	{
		damageNumber->transform.localPosition.x += damageOffsetDummy->transform.localPosition.x;
		damageNumber->transform.localPosition.y += damageOffsetDummy->transform.localPosition.y;
	}
	damageNumber->color.w = damageNumberInit->color.w;
	K2_LOG("ResetIcon color.w = %f", damageNumber->color.w);
	damageNumber->isDraw = false;
	K2_LOG("ResetIcon isDraw = %d", (int)damageNumber->isDraw);


	// クリティカルUI
	const uint32_t criticalAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/criticalHit0"),
		Hash32("InGame_Boss_HitDamage/criticalHit1"),
		Hash32("InGame_Boss_HitDamage/criticalHit2"),
	};
	// クリティカルUI：オフセット
	const uint32_t criticalOffsetAssets[DAMAGE_POOL_SIZE] = {
		Hash32("InGame_Boss_HitDamage/CriticalOffset0"),
		Hash32("InGame_Boss_HitDamage/CriticalOffset1"),
		Hash32("InGame_Boss_HitDamage/CriticalOffset2"),
	};

	// ボスのクリティカルダメ
	auto* criticalDamage = GetUI<UIIcon>(criticalAssets[poolIndex]);
	auto* criticaldamageInit = GetUI<UIDummy>(Hash32("InGame_Boss_HitDamage/InitCriticalDamage"));
	auto* criticalOffsetDummy = GetUI<UIDummy>(criticalOffsetAssets[poolIndex]);
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(criticalAssets[poolIndex]))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	criticalDamage->transform.localPosition = criticaldamageInit->transform.localPosition;
	// .jsonのオフセット
	if (criticalOffsetDummy)
	{
		criticalDamage->transform.localPosition.x += criticalOffsetDummy->transform.localPosition.x;
		criticalDamage->transform.localPosition.y += criticalOffsetDummy->transform.localPosition.y;
	}
	criticalDamage->transform.localScale = criticaldamageInit->transform.localScale;
	criticalDamage->color.w = criticaldamageInit->color.w;
	criticalDamage->isDraw = false;
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
