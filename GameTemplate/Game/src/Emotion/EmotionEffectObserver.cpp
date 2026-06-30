#include "stdafx.h"
#include "EmotionEffectObserver.h"
#include "EmotionSystem.h"
#include "src/Actor/Player.h"
#include "src/Effect/EffectManager.h"
#include "src/Effect/Types.h"
#include "src/Core/ParameterManager.h"

void EmotionEffectObserver::Init(Player* player, EmotionSystem* emotionSystem)
{
    player_ = player;

    // JSONパラメーターからエフェクトスケールを取得する
    const auto* param = ParameterManager::Get().GetEmotionParam();
    if (param)
    {
        buffEffectScale_    = param->buffEffectScale;
        debuffEffectScale_  = param->debuffEffectScale;
        buffEffectOffset_   = param->buffEffectOffset;
        debuffEffectOffset_ = param->debuffEffectOffset;
    }

    // 自分をオブザーバーとして登録する
    emotionSystem->AddObserver(this);
}

void EmotionEffectObserver::Update()
{
    if (!player_ || currentEffectHandle_ == INVALID_EFFECT_HANDLE) { return; }

    effectFrameTimer_++;
    if (effectFrameTimer_ >= kEffectFollowMaxFrames)
    {
        // エフェクト再生時間を超えたとみなし追従を終了する
        // （K2Engine が DeleteGO 済みのポインタへのアクセスを防ぐため）
        currentEffectHandle_ = INVALID_EFFECT_HANDLE;
        return;
    }

    // 再生中エフェクトをプレイヤー位置に追従させる
    EffectManager::Get().SetEffectPosition(currentEffectHandle_, player_->GetTransformPosition());
}

void EmotionEffectObserver::OnEmotionChanged(int oldLevel, int newLevel)
{
    if (!player_) { return; }

    Vector3 pos = player_->GetTransformPosition();

    effectFrameTimer_ = 0;

    if (newLevel > oldLevel)
    {
        // レベルが上がった → Buff エフェクト
        float s = buffEffectScale_;
        currentEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Buff,
            pos + buffEffectOffset_,
            Quaternion::Identity,
            { s, s, s }
        );
    }
    else
    {
        // レベルが下がった → Debuff エフェクト
        float s = debuffEffectScale_;
        currentEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Debuff,
            pos + debuffEffectOffset_,
            Quaternion::Identity,
            { s, s, s }
        );
    }
}
