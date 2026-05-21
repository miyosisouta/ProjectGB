#pragma once
#include "src/Actor/ActorStatus.h"
#include <random>
#include <algorithm>

namespace
{
    inline int Calculate(const ActorStatus* attacker, float motionValue)
    {
        if (!attacker) return 0;

        // 1. 基礎ダメージ (モーション値は % 扱いとする。100.0f で 1.0倍)
        float baseDamage = attacker->GetAttack() * (motionValue / 100.0f);

        // 2. クリティカル判定
        static std::random_device rd;
        static std::mt19937 gen(rd());
        std::uniform_int_distribution<> disRate(0, 99);

        if (disRate(gen) < attacker->GetCritical()) {
            baseDamage *= attacker->GetCriticalDamageMultiplier();
        }

        // 3. 乱数ブレ (±5%)
        std::uniform_real_distribution<float> disVariance(0.95f, 1.05f);
        baseDamage *= disVariance(gen);

        // 最低1ダメージ保証
        return static_cast<int>(max(1.0f, baseDamage));
    }
}