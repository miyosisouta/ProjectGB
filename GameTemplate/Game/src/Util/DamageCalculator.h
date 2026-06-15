#pragma once
#include "src/Actor/ActorStatus.h"
#include <random>
#include <algorithm>

// 計算結果をまとめる構造体
struct DamageResult
{
    int  damage = 0;
    bool isCritical = false;
};

namespace
{
    inline DamageResult Calculate(const ActorStatus* attacker, float motionValue)
    {
        DamageResult result;
        if (!attacker) return result;

        // 1. 基礎ダメージ
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

        result.damage = static_cast<int>(max(1.0f, baseDamage));
        return result;
    }
}