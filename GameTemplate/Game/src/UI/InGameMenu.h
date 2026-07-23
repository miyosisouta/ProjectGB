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
	std::unique_ptr<UIAnimationSequence> abilitySkillIconScaleSequence_ = nullptr; //!< 特殊スキルアイコンの拡縮アニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> abilitySkillButtonIconScaleSequence_ = nullptr; //!< 特殊スキルボタンアイコンの拡縮アニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> bossHitHPPositionSequence_ = nullptr; //!< ボスHPバーのヒット時位置アニメーションシーケンス
	std::unique_ptr<TaskSchedulerSystem> bossHitDamageScheduler_[DAMAGE_POOL_SIZE]; //!< ボス被ダメージ表示のプールごとのタイマー
	std::unique_ptr<TaskSchedulerSystem> playerDamageScheduler_ = nullptr; //!< プレイヤー被ダメージ表示のタイマー
	std::unique_ptr<TaskSchedulerSystem> bossCriticalHitDamageScheduler_[DAMAGE_POOL_SIZE]; //!< ボスクリティカル被ダメージ表示のプールごとのタイマー

	/**
	 * ボスの感情バフ/デバフアイコン
	 * enumの値が小さいほどHPバー右端の起点(origin)に近い側に並ぶ
	 */
	enum class EmotionBuffKind : int
	{
		DamageTakenMul = 0, //!< 被ダメージ倍率アイコン
		AttackSpeed    = 1, //!< 攻撃速度アイコン
		AttackPower    = 2, //!< 攻撃力アイコン
	};
	static const int EmotionBuffKindCount = 3;

	/** 直前フレームの感情レベル（変化検出用。初期値はEmotionLevel::Normalと同じ0） */
	int lastEmotionLevel_ = 0;
	/** 各バフ種別が直前フレームで表示されていたか（表示/非表示の切り替わり検出用） */
	bool emotionBuffWasActive_[EmotionBuffKindCount] = { false, false, false };
	/** 表示時のポップ演出（拡大->縮小） */
	std::unique_ptr<UIAnimationSequence> emotionIconPopSequence_[EmotionBuffKindCount];
	/** 非表示時のフェードアウト演出 */
	std::unique_ptr<UIAnimationSequence> emotionIconFadeOutSequence_[EmotionBuffKindCount];

	GaugeRender staminaGauge_; //!< スタミナゲージのレンダー
	bool isExhausted_ = false; //!< 枯渇状態キャッシュ（Render側でも参照するため保持）


private:
	/** プレイヤー被ダメージ表示用タスクスケジューラーを生成する */
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

	/** ボスの感情バフ/デバフアイコンの表示・アニメーションを更新する */
	void UpdateEmotionBuffIcon(const int currentLevel);


public:
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext& rc) override;
	/** UIロジックの初期化 */
	void InitializeLogic() override;
};