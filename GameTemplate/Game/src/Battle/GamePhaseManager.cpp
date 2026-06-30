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

    // HP50%閾値：ボス激怒＋プレイヤー強制Debuff3
    if (!phase50Triggered_ && hpRatio <= 0.5f)
    {
        phase50Triggered_ = true;
        OnBossPhase50();
    }

    // HP25%閾値：プレイヤー強制Buff3
    // 50%の閾値を先に処理するため、25%はその後に独立してチェックする
    if (!phase25Triggered_ && hpRatio <= 0.25f)
    {
        phase25Triggered_ = true;
        OnBossPhase25();
    }
}

void GamePhaseManager::OnBossPhase50()
{
    // プレイヤーを最大デバフに強制セット（激怒したボスに圧倒されている状態）
    emotionSystem_->ForceSetLevel(EmotionLevel::Debuff3);

    // ボスを激怒状態にしてステータスを上昇させる
    auto* bossStatus = boss_->GetStatus()->As<BossStatus>();
    if (bossStatus) { bossStatus->SetEnraged(true); }

    // todo for test : 値の視覚化（HP50%でDebuff3になり、ボスが激怒したことを確認）
}

void GamePhaseManager::OnBossPhase25()
{
    // プレイヤーを最大バフに強制セット（瀕死のボスを目の前に奮起している状態）
    emotionSystem_->ForceSetLevel(EmotionLevel::Buff3);

    // todo for test : 値の視覚化（HP25%でBuff3になったことを確認）
}
