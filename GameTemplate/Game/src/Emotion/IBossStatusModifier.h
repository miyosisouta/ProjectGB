#pragma once

// ボスのステータスへの倍率修正を提供するインターフェース
// EmotionSystem 以外の修正要因（今後増える場合）も同じ口で差し込める
// BossStatus は IBossStatusModifier* のリストを持ち、攻撃力・攻撃速度・被ダメージ取得時に全修正子を適用する
class IBossStatusModifier
{
public:
    virtual ~IBossStatusModifier() = default;

    /** 攻撃力倍率を返す（1.0f = 等倍） */
    virtual float GetAttackMul() const = 0;

    /** 攻撃速度倍率を返す（1.0f = 等倍。値が大きいほどクールダウンが短くなる） */
    virtual float GetAttackSpeedMul() const = 0;

    /** 被ダメージ倍率を返す（1.0f = 等倍） */
    virtual float GetDamageTakenMul() const = 0;

    /** アニメーション再生速度倍率を返す（1.0f = 等倍。攻撃速度とは独立に調整できる演出用の値） */
    virtual float GetAnimationSpeedMul() const = 0;

    /** エフェクト再生速度倍率を返す（1.0f = 等倍。攻撃速度とは独立に調整できる演出用の値） */
    virtual float GetEffectSpeedMul() const = 0;
};
