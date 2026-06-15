/**
 * InGameMenu.h
 * インゲームメニュー
 */
#pragma once
#include "Menu.h"


class InGameMenu : public MenuBase
{
private:
	std::unique_ptr<UIAnimationSequence> abilitySkillIconScaleSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> abilitySkillButtonIconScaleSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> bossHitHPPositionSequence_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> bossHitDamageScheduler_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> playerDamageScheduler_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> bossCriticalHitDamageScheduler_ = nullptr;

	// スタミナの情報を取得するためにいるプレイヤーステートとゲージレンダー
	GaugeRender staminaGauge_;

private:
	/** ボスの被ダメージ */
	float bossHitDamage_ = 0.0f;
	/** ボスのクリティカル被ダメージ */
	float bossCriticalHitDamage_ = 0.0f;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void CreatePlayerDamageScheduler();

	/** ボスが被ダメを受けた時のアニメーション */
	void BossHitAnimationScheduler();
	/** ボスの被ダメアニメーションの初期化 */
	void BossHitAnimationResetIcon();
	/** ボスの被ダメ時にクリティカルが発生した時のアニメーション */
	void BossCriticalHitAnimationScheduler();

	/** ボタンの情報(表示) */
	void UpdateButtonWord(const uint32_t enAction);

public:
	/** 攻撃力の設定(= ボスの被ダメージ) */
	void SetBossHitDamage(const float bossHitDamage) { bossHitDamage_ = bossHitDamage; }

	/** 攻撃力の設定(= ボスの被ダメージ) */
	void SetBossCriticalHitDamage(const float bossCriticalHitDamage) { bossCriticalHitDamage_ = bossCriticalHitDamage; }
};