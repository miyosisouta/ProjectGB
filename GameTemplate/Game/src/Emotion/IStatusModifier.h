#pragma once

// ステータスへの倍率修正を提供するインターフェース
// 感情システム以外のバフ（装備強化・スキル効果など）も将来的に同じ口で差し込める
// PlayerStatus は IStatusModifier* のリストを持ち、GetAttack/GetSpeed 時に全修正子を適用する
class IStatusModifier
{
public:
    virtual ~IStatusModifier() = default;

    /** 攻撃力倍率を返す（1.0f = 等倍） */
    virtual float GetAttackMul() const = 0;

    /** 移動速度倍率を返す（1.0f = 等倍） */
    virtual float GetSpeedMul()  const = 0;
};
