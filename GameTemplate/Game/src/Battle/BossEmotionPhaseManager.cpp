#include "stdafx.h"
#include "BossEmotionPhaseManager.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/ActorStatus.h"
#include "src/Emotion/EmotionSystem.h"

void BossEmotionPhaseManager::Init(BossCharacter* boss, EmotionSystem* emotionSystem)
{
    // HP監視対象のボスを保持
    boss_          = boss;
    // 強制変更先の感情システムを保持
    emotionSystem_ = emotionSystem;
}

void BossEmotionPhaseManager::Update()
{

}

void BossEmotionPhaseManager::ForceApplyPhase50Buff()
{
    // 感情システムが未初期化なら何もしない
    if (!emotionSystem_) { return; }

    // 感情レベルを最大のBuff3に強制変更
    emotionSystem_->ForceSetLevel(EmotionLevel::Buff3);
}

void BossEmotionPhaseManager::ForceApplyPhase25Debuff()
{
    // 感情システムが未初期化なら何もしない
    if (!emotionSystem_) { return; }

    // 感情レベルを最大のDebuff3に強制変更
    emotionSystem_->ForceSetLevel(EmotionLevel::Debuff3);
}
