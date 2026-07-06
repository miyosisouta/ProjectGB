#pragma once
#include "IEmotionObserver.h"
#include "src/Effect/EffectManager.h"

class BossCharacter;
class EmotionSystem;

// ボスの感情（強気⇔動揺）の変化を受け取り、上昇/下降に応じてエフェクトを再生するオブザーバー
// EmotionSystem 本体は EffectManager も BossCharacter も知らなくて済む（Observerパターン）
class EmotionEffectObserver : public IEmotionObserver
{
private:
    BossCharacter* boss_                = nullptr;              //!< エフェクト位置の基点
    float        buffEffectScale_       = 1.0f;                 //!< 強気化エフェクトのスケール（JSONで設定）
    float        debuffEffectScale_     = 1.0f;                 //!< 動揺エフェクトのスケール（JSONで設定）
    float        buffEffectOffsetY_     = 0.0f;                 //!< 強気化エフェクトのボス位置からのYオフセット（JSONで設定。XZはボス座標のまま）
    float        debuffEffectOffsetY_   = 0.0f;                 //!< 動揺エフェクトのボス位置からのYオフセット（JSONで設定。XZはボス座標のまま）
    EffectHandle currentEffectHandle_   = INVALID_EFFECT_HANDLE; //!< 再生中エフェクトのハンドル（追従用）
    int          effectFrameTimer_      = 0;                     //!< エフェクト開始からの経過フレーム数
    // K2Engine は非ループエフェクト終了時に EffectEmitter を DeleteGO するため、
    // それ以降に SetEffectPosition を呼ぶとダングリングポインタ参照になりクラッシュする。
    // このフレーム数を超えたらハンドルを破棄して追従を止める（エフェクト時間より長めに設定）。
    static constexpr int kEffectFollowMaxFrames = 180; //!< 追従上限フレーム数（約3秒 @60fps）

public:
    EmotionEffectObserver()  = default;
    ~EmotionEffectObserver() = default;

    /** 初期化。BattleManager の Playing 開始時に呼ぶ */
    void Init(BossCharacter* boss, EmotionSystem* emotionSystem);

    /** 毎フレーム呼ぶ。再生中エフェクトをボスに追従させる */
    void Update();

    /** 感情のレベルが変化したときに呼ばれる（IEmotionObserver の実装） */
    void OnEmotionChanged(int oldLevel, int newLevel) override;
};
