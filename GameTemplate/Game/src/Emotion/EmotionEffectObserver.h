#pragma once
#include "IEmotionObserver.h"
#include "src/Effect/EffectManager.h"

class BossCharacter;
class EmotionSystem;

/**
 * ボスの感情（Buff⇔Debuff）の変化を受け取り、上昇/下降に応じてエフェクトを再生するオブザーバー
 * EmotionSystem 本体は EffectManager も BossCharacter も知らなくて済む（Observerパターン）
 */
class EmotionEffectObserver : public IEmotionObserver
{
private:
    BossCharacter* boss_                = nullptr;               //!< エフェクト位置の基点
    float        buffEffectScale_       = 1.0f;                  //!< Buffエフェクトのスケール
    float        debuffEffectScale_     = 1.0f;                  //!< Debuffエフェクトのスケール
    float        buffEffectOffsetY_     = 0.0f;                  //!< Buffエフェクトのボス位置からのYオフセット
    float        debuffEffectOffsetY_   = 0.0f;                  //!< Debuffエフェクトのボス位置からのYオフセット
    float        buffSeVolumeScale_     = 1.0f;                  //!< BuffSEの追加音量倍率
    float        debuffSeVolumeScale_   = 1.0f;                  //!< DebuffSEの追加音量倍率
    EffectHandle currentEffectHandle_   = INVALID_EFFECT_HANDLE; //!< 再生中エフェクトのハンドル
    int          effectFrameTimer_      = 0;                     //!< エフェクト開始からの経過フレーム数

    bool  hasBuffOffsetYOverride_   = false; //!< 次回のBuffエフェクトだけYオフセットを上書きするか
    float buffOffsetYOverride_      = 0.0f;  //!< 上書き先のYオフセット値（1回使ったら自動的に解除する）
    bool  hasDebuffOffsetYOverride_ = false; //!< 次回のDebuffエフェクトだけYオフセットを上書きするか
    float debuffOffsetYOverride_    = 0.0f;  //!< 上書き先のYオフセット値（1回使ったら自動的に解除する）

    // K2Engine は非ループエフェクト終了時に EffectEmitter を DeleteGO するため、
    // それ以降に SetEffectPosition を呼ぶとダングリングポインタ参照になりクラッシュする。
    // このフレーム数を超えたらハンドルを破棄して追従を止める（エフェクト時間より長めに設定）。
    static constexpr int kEffectFollowMaxFrames = 180; //!< 追従上限フレーム数

public:
    /** コンストラクタ */
    EmotionEffectObserver()  = default;
    /** デストラクタ */
    ~EmotionEffectObserver() = default;

    /** 初期化。BattleManager の Playing 開始時に呼ぶ */
    void Init(BossCharacter* boss, EmotionSystem* emotionSystem);

    /** 毎フレーム呼ぶ。再生中エフェクトをボスに追従させる */
    void Update();

    /** 感情のレベルが変化したときに呼ばれる（IEmotionObserver の実装） */
    void OnEmotionChanged(int oldLevel, int newLevel) override;

    /**
     * 次に発生するBuffエフェクト1回分だけ、Yオフセットを上書きする。
     * カットシーンなど特定の演出だけ位置を変えたい場合に、エフェクト発火の直前に呼ぶ。
     * 使用後は自動的に解除され、以降は通常のbuffEffectOffsetY_に戻る。
     */
    void SetNextBuffEffectOffsetY(float offsetY)
    {
        hasBuffOffsetYOverride_ = true;
        buffOffsetYOverride_ = offsetY;
    }

    /**
     * 次に発生するDebuff1回分だけ、Yオフセットを上書きする。
     * カットシーンなど特定の演出だけ位置を変えたい場合に、エフェクト発火の直前に呼ぶ。
     * 使用後は自動的に解除され、以降は通常のdebuffEffectOffsetY_に戻る。
     */
    void SetNextDebuffEffectOffsetY(float offsetY)
    {
        hasDebuffOffsetYOverride_ = true;
        debuffOffsetYOverride_ = offsetY;
    }
};
