#include "stdafx.h"
#include "GamePhaseManager.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/ActorStatus.h"
#include "src/Emotion/EmotionSystem.h"

void GamePhaseManager::Init(BossCharacter* boss, EmotionSystem* emotionSystem)
{
    boss_          = boss;
    emotionSystem_ = emotionSystem;
}

void GamePhaseManager::Update()
{
    if (!boss_ || !emotionSystem_) { return; }

    auto* bossStatus = boss_->GetStatus()->As<BossStatus>();
    if (!bossStatus) { return; }

    float hpRatio = static_cast<float>(bossStatus->GetHP())
                  / static_cast<float>(bossStatus->GetMaxHP());

    // HP50%閾値：後がないという焦りから全力を出す（強制Buff3）
    if (!phase50Triggered_ && hpRatio <= 0.5f)
    {
        phase50Triggered_ = true;
        OnBossPhase50();
    }

    // HP25%閾値：追い詰められて冷静さを欠く（強制Debuff3）
    // 50%の閾値を先に処理するため、25%はその後に独立してチェックする
    if (!phase25Triggered_ && hpRatio <= 0.25f)
    {
        phase25Triggered_ = true;
        OnBossPhase25();
    }
}

void GamePhaseManager::OnBossPhase50()
{
    // 後がないという焦りから全力を出す（興奮状態）
    emotionSystem_->ForceSetLevel(EmotionLevel::Buff3);
}

void GamePhaseManager::OnBossPhase25()
{
    // 追い詰められていることへの動揺。冷静さを欠いて隙ができる
    emotionSystem_->ForceSetLevel(EmotionLevel::Debuff3);
}
