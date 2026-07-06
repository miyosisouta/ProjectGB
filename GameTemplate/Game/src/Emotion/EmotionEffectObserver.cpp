#include "stdafx.h"
#include "EmotionEffectObserver.h"
#include "EmotionSystem.h"
#include "src/Actor/BossCharacter.h"
#include "src/Effect/EffectManager.h"
#include "src/Effect/Types.h"
#include "src/Core/ParameterManager.h"

void EmotionEffectObserver::Init(BossCharacter* boss, EmotionSystem* emotionSystem)
{
    boss_ = boss;

    // JSONパラメーターからエフェクトスケールを取得する
    const auto* param = ParameterManager::Get().GetEmotionParam();
    if (param)
    {
        buffEffectScale_     = param->buffEffectScale;
        debuffEffectScale_   = param->debuffEffectScale;
        buffEffectOffsetY_   = param->buffEffectOffsetY;
        debuffEffectOffsetY_ = param->debuffEffectOffsetY;
    }

    // 自分をオブザーバーとして登録する
    emotionSystem->AddObserver(this);
}

void EmotionEffectObserver::Update()
{
    if (!boss_ || currentEffectHandle_ == INVALID_EFFECT_HANDLE) { return; }

    effectFrameTimer_++;
    if (effectFrameTimer_ >= kEffectFollowMaxFrames)
    {
        // エフェクト再生時間を超えたとみなし追従を終了する
        // （K2Engine が DeleteGO 済みのポインタへのアクセスを防ぐため）
        currentEffectHandle_ = INVALID_EFFECT_HANDLE;
        return;
    }

    // 再生中エフェクトをボス位置に追従させる
    EffectManager::Get().SetEffectPosition(currentEffectHandle_, boss_->GetTransformPosition());
}

void EmotionEffectObserver::OnEmotionChanged(int oldLevel, int newLevel)
{
    if (!boss_) { return; }

    Vector3 pos = boss_->GetTransformPosition();

    effectFrameTimer_ = 0;

    if (newLevel > oldLevel)
    {
        // レベルが上がった → 強気化エフェクト
        float s = buffEffectScale_;
        pos.y += buffEffectOffsetY_;
        currentEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Buff,
            pos,
            Quaternion::Identity,
            { s, s, s }
        );
    }
    else
    {
        // レベルが下がった → 動揺エフェクト
        float s = debuffEffectScale_;
        pos.y += debuffEffectOffsetY_;
        currentEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Debuff,
            pos,
            Quaternion::Identity,
            { s, s, s }
        );
    }
}
