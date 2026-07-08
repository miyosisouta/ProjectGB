#include "stdafx.h"
#include "BossEmotionPhaseManager.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/ActorStatus.h"
#include "src/Emotion/EmotionSystem.h"

void BossEmotionPhaseManager::Init(BossCharacter* boss, EmotionSystem* emotionSystem)
{
    boss_          = boss;
    emotionSystem_ = emotionSystem;
}

void BossEmotionPhaseManager::Update()
{
    // HP50%/25%閾値のBuff/Debuff適用はここでは行わない。
    // BattleManager側のボスHP閾値カットシーンの演出タイミングに合わせて
    // ForceApplyPhase50Buff() / ForceApplyPhase25Debuff() を呼ぶ
}

void BossEmotionPhaseManager::ForceApplyPhase50Buff()
{
    if (!emotionSystem_) { return; }

    // 後がないという焦りから全力を出す（興奮状態）
    emotionSystem_->ForceSetLevel(EmotionLevel::Buff3);
}

void BossEmotionPhaseManager::ForceApplyPhase25Debuff()
{
    if (!emotionSystem_) { return; }

    // 追い詰められていることへの動揺。冷静さを欠いて隙ができる
    emotionSystem_->ForceSetLevel(EmotionLevel::Debuff3);
}
