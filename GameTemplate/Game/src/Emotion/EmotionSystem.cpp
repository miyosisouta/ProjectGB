#include "stdafx.h"
#include "EmotionSystem.h"
#include "IEmotionObserver.h"
#include "src/Core/ParameterManager.h"
#include <algorithm>

EmotionSystem::EmotionSystem()
{
    InitModifierTable();
}

void EmotionSystem::Init()
{
    // JSONパラメーターから倍率テーブルを上書きロードする
    // コンストラクタ時点では ParameterManager が未初期化なので PlayerStatus::Init() から呼ぶ
    const auto* param = ParameterManager::Get().GetEmotionParam();
    if (!param) { return; }
    for (int i = 0; i < 7; i++)
    {
        modifierTable_[i].attackMul = param->modifiers[i].attackMul;
        modifierTable_[i].speedMul  = param->modifiers[i].speedMul;
    }
}

void EmotionSystem::InitModifierTable()
{
    // インデックス = static_cast<int>(level) + 3
    modifierTable_[0] = { 0.5f, 0.6f };  // Debuff3: 攻撃0.5倍 速度0.6倍
    modifierTable_[1] = { 0.8f, 0.8f };  // Debuff2: 攻撃0.8倍 速度0.8倍
    modifierTable_[2] = { 0.9f, 0.8f };  // Debuff1: 攻撃0.9倍 速度0.8倍
    modifierTable_[3] = { 1.0f, 1.0f };  // Normal:  等倍
    modifierTable_[4] = { 1.1f, 1.0f };  // Buff1:   攻撃1.1倍
    modifierTable_[5] = { 1.2f, 1.0f };  // Buff2:   攻撃1.2倍
    modifierTable_[6] = { 1.5f, 1.0f };  // Buff3:   攻撃1.5倍
}

void EmotionSystem::OnJustAvoid()
{
    int current = static_cast<int>(currentLevel_);

    // デバフ中はジャスト回避で帳消し→即 Normal に戻す（1段階ずつではない）
    // 仕様：「その前にデバフがかかっている場合は、デバフが帳消しになり通常状態になる」
    if (current < 0)
    {
        ChangeLevel(EmotionLevel::Normal);
        return;
    }

    // 通常/バフ中は1段階上昇（上限 Buff3 でクランプ）
    // windows.h の min マクロと衝突するため () で囲んで展開を防ぐ
    int next = (std::min)(current + 1, LevelMax);
    ChangeLevel(static_cast<EmotionLevel>(next));
}

void EmotionSystem::OnStrongAttackHit()
{
    int current = static_cast<int>(currentLevel_);

    // バフ中は強攻撃被弾で帳消し→即 Normal に戻す（1段階ずつではない）
    if (current > 0)
    {
        ChangeLevel(EmotionLevel::Normal);
        return;
    }

    // 通常/デバフ中は1段階下降（下限 Debuff3 でクランプ）
    // windows.h の max マクロと衝突するため () で囲んで展開を防ぐ
    int next = (std::max)(current - 1, LevelMin);
    ChangeLevel(static_cast<EmotionLevel>(next));
}

void EmotionSystem::ForceSetLevel(EmotionLevel level)
{
    // HP閾値などゲーム側からの強制変更。ChangeLevel を通して Observer にも通知する
    ChangeLevel(level);
}

float EmotionSystem::GetAttackMul() const
{
    int idx = static_cast<int>(currentLevel_) + 3;
    return modifierTable_[idx].attackMul;
}

float EmotionSystem::GetSpeedMul() const
{
    int idx = static_cast<int>(currentLevel_) + 3;
    return modifierTable_[idx].speedMul;
}

void EmotionSystem::AddObserver(IEmotionObserver* observer)
{
    observers_.push_back(observer);
}

void EmotionSystem::RemoveObserver(IEmotionObserver* observer)
{
    observers_.erase(
        std::remove(observers_.begin(), observers_.end(), observer),
        observers_.end()
    );
}

void EmotionSystem::ChangeLevel(EmotionLevel newLevel)
{
    // 同レベルへの変更は何もしない（Observer への無駄な通知を防ぐ）
    if (newLevel == currentLevel_) { return; }

    int oldLevelInt = static_cast<int>(currentLevel_);
    int newLevelInt = static_cast<int>(newLevel);
    currentLevel_ = newLevel;

    // UI・エフェクトへ通知（EmotionSystem 自身は UI の実装を知らない）
    for (auto* obs : observers_)
    {
        obs->OnEmotionChanged(oldLevelInt, newLevelInt);
    }
}
