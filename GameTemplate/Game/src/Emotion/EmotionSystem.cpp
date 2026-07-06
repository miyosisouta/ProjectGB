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
    // コンストラクタ時点では ParameterManager が未初期化なので BossStatus::Init() から呼ぶ
    const auto* param = ParameterManager::Get().GetEmotionParam();
    if (!param) { return; }
    for (int i = 0; i < 7; i++)
    {
        modifierTable_[i].attackMul         = param->modifiers[i].attackMul;
        modifierTable_[i].attackSpeedMul    = param->modifiers[i].attackSpeedMul;
        modifierTable_[i].damageTakenMul    = param->modifiers[i].damageTakenMul;
        modifierTable_[i].animationSpeedMul = param->modifiers[i].animationSpeedMul;
        modifierTable_[i].effectSpeedMul    = param->modifiers[i].effectSpeedMul;
    }
}

void EmotionSystem::InitModifierTable()
{
    // インデックス = static_cast<int>(level) + 3
    // JSON未読み込み時のフォールバック値。実際の効果量は EmotionParameter.json 側で調整する
    // 攻撃力・攻撃速度は「強気になるほど上がる」、被ダメージは「動揺するほど上がる」軸で設計している
    // animationSpeedMul/effectSpeedMul は演出用の値。デフォルトではattackSpeedMulと同じ値にしているが、JSON側で独立に調整できる
    modifierTable_[0] = { 1.15f, 0.85f, 1.5f, 0.85f, 0.85f };  // Debuff3: 攻撃力上昇(小) 攻撃速度Down(小) 被ダメージUp(大)
    modifierTable_[1] = { 1.05f, 1.0f,  1.2f, 1.0f,  1.0f  };  // Debuff2: 攻撃力上昇(微) 被ダメージUp(小)
    modifierTable_[2] = { 1.0f,  1.0f,  1.1f, 1.0f,  1.0f  };  // Debuff1: 被ダメージUp(微)
    modifierTable_[3] = { 1.0f,  1.0f,  1.0f, 1.0f,  1.0f  };  // Normal:  等倍
    modifierTable_[4] = { 1.1f,  1.0f,  1.0f, 1.0f,  1.0f  };  // Buff1:   攻撃力Up(微)
    modifierTable_[5] = { 1.2f,  1.05f, 1.0f, 1.05f, 1.05f };  // Buff2:   攻撃力Up(小) 攻撃速度Up(微)
    modifierTable_[6] = { 1.5f,  1.15f, 0.8f, 1.15f, 1.15f };  // Buff3:   攻撃力Up(大) 攻撃速度Up(小) 被ダメージDown(小)
}

void EmotionSystem::OnAttackHitPlayer()
{
    int current = static_cast<int>(currentLevel_);

    // 動揺中（Debuff）に攻撃が当たったら1段階だけ動揺が晴れる。通常/強気中は1段階強気になる（上限 Buff3 でクランプ）
    // windows.h の min マクロと衝突するため () で囲んで展開を防ぐ
    int next = (std::min)(current + 1, LevelMax);
    ChangeLevel(static_cast<EmotionLevel>(next));
}

void EmotionSystem::OnJustAvoidedByPlayer()
{
    int current = static_cast<int>(currentLevel_);

    // 強気中（Buff）にジャスト回避されたら、調子の良さが帳消しになり即 Normal に戻す（1段階ずつではない）
    // 仕様：「ボスにバフがついているときにジャスト回避を行った場合、バフの効果はすべて消されて通常状態になる」
    if (current > 0)
    {
        ChangeLevel(EmotionLevel::Normal);
        return;
    }

    // 通常/動揺中は1段階動揺が深まる（下限 Debuff3 でクランプ）
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

float EmotionSystem::GetAttackSpeedMul() const
{
    int idx = static_cast<int>(currentLevel_) + 3;
    return modifierTable_[idx].attackSpeedMul;
}

float EmotionSystem::GetDamageTakenMul() const
{
    int idx = static_cast<int>(currentLevel_) + 3;
    return modifierTable_[idx].damageTakenMul;
}

float EmotionSystem::GetAnimationSpeedMul() const
{
    int idx = static_cast<int>(currentLevel_) + 3;
    return modifierTable_[idx].animationSpeedMul;
}

float EmotionSystem::GetEffectSpeedMul() const
{
    int idx = static_cast<int>(currentLevel_) + 3;
    return modifierTable_[idx].effectSpeedMul;
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
