#pragma once
#include "src/Actor/ActorStatus.h"
#include <random>
#include <algorithm>

/** ダメージ計算結果 */
struct DamageResult
{
    int  damage = 0; //!< 最終的なダメージ量
    bool isCritical = false; //!< クリティカルが発生したか
};

namespace
{
    /** 攻撃者のステータスとモーション値からダメージを計算する */
    inline DamageResult Calculate(const ActorStatus* attacker, float motionValue)
    {
        DamageResult result;
        if (!attacker) return result;

        // 1. 基礎ダメージ (モーション値は % 扱いとする。100.0f で 1.0倍)
        float baseDamage = attacker->GetAttack() * (motionValue / 100.0f);

        // 2. クリティカル判定
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> disRate(0, 99);

        if (disRate(gen) < attacker->GetCritical()) {
            baseDamage *= attacker->GetCriticalDamageMultiplier();
            result.isCritical = true; // クリティカルフラグを立てる
        }

        // 3. 乱数ブレ (±5%)
        std::uniform_real_distribution<float> disVariance(0.95f, 1.05f);
        baseDamage *= disVariance(gen);

        // 最低1ダメージ保証
        result.damage = static_cast<int>(max(1.0f, baseDamage));
        return result;
    }
}