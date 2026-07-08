#include "stdafx.h"
#include "EmotionEffectObserver.h"
#include "EmotionSystem.h"
#include "src/Actor/BossCharacter.h"
#include "src/Effect/EffectManager.h"
#include "src/Effect/Types.h"
#include "src/Core/ParameterManager.h"
#include "src/Sound/SoundManager.h"

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
        buffSeVolumeScale_   = param->buffSeVolumeBoost;
        debuffSeVolumeScale_ = param->debuffSeVolumeBoost;
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
        // 上書き指定があればその回だけ使い、使ったら解除する（通常はJSON設定値のbuffEffectOffsetY_を使う）
        if (hasBuffOffsetYOverride_)
        {
            pos.y += buffOffsetYOverride_;
            hasBuffOffsetYOverride_ = false;
        }
        else
        {
            pos.y += buffEffectOffsetY_;
        }
        currentEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Buff,
            pos,
            Quaternion::Identity,
            { s, s, s }
        );
        SoundManager::Get().PlaySE(enSoundKind_InGame_Buff, false, false, buffSeVolumeScale_);
    }
    else
    {
        // レベルが下がった → 動揺エフェクト
        float s = debuffEffectScale_;
        // 上書き指定があればその回だけ使い、使ったら解除する（通常はJSON設定値のdebuffEffectOffsetY_を使う）
        if (hasDebuffOffsetYOverride_)
        {
            pos.y += debuffOffsetYOverride_;
            hasDebuffOffsetYOverride_ = false;
        }
        else
        {
            pos.y += debuffEffectOffsetY_;
        }
        currentEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Debuff,
            pos,
            Quaternion::Identity,
            { s, s, s }
        );
        // Debuff.wavは収録音量が小さいため、通常のボリューム計算に追加で倍率をかけて底上げする（EmotionParameter.jsonのdebuffSeVolumeBoostで調整）
        SoundManager::Get().PlaySE(enSoundKind_InGame_Debuff, false, false, debuffSeVolumeScale_);
    }
}
