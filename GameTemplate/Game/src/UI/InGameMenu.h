/**
 * InGameMenu.h
 * インゲームメニュー
 */
#pragma once
#include "Menu.h"


class InGameMenu : public MenuBase
{
private:
	/** ボスの被ダメージ表示を３個表示する変数 */
	static const int DAMAGE_POOL_SIZE = 3;
	/** 次に使う番号 */
	int damagePoolIndex_ = 0;
	/** 被ダメージ表示のクールダウン */
	float damageNotifyCoolDown_ = 0.0;
	/** 同一攻撃とみなす時間 */
	static constexpr float damageNotifyCoolDownTime = 0.2f;

private:
	std::unique_ptr<UIAnimationSequence> abilitySkillIconScaleSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> abilitySkillButtonIconScaleSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> bossHitHPPositionSequence_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> bossHitDamageScheduler_[DAMAGE_POOL_SIZE];
	std::unique_ptr<TaskSchedulerSystem> playerDamageScheduler_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> bossCriticalHitDamageScheduler_[DAMAGE_POOL_SIZE];

	// スタミナの情報を取得するためにいるプレイヤーステートとゲージレンダー
	GaugeRender staminaGauge_;
	// 枯渇状態キャッシュ（Render側でも参照するため保持）
	bool isExhausted_ = false;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void CreatePlayerDamageScheduler();

	/** ボスが被ダメを受けた時のアニメーション */
	void BossHitAnimationScheduler(int poolIndex);
	/** ボスの被ダメアニメーションの初期化 */
	void BossHitAnimationResetIcon(int poolIndex);
	/** ボスの被ダメ時にクリティカルが発生した時のアニメーション */
	void BossCriticalHitAnimationScheduler(int poolIndex);
	/** ボスの被ダメージ表示 */
	void UpdateBossHitDamage(const DamageNotify& notify);

	/** ボタンの情報(表示) */
	void UpdateButtonWord(const uint32_t enAction);
};