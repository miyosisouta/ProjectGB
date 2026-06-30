#pragma once
#include "IEmotionObserver.h"
#include "src/Effect/EffectManager.h"

class Player;
class EmotionSystem;

// 感情レベルの変化を受け取り、上昇/下降に応じてエフェクトを再生するオブザーバー
// EmotionSystem 本体は EffectManager も Player も知らなくて済む（Observerパターン）
class EmotionEffectObserver : public IEmotionObserver
{
private:
    Player*      player_               = nullptr;               //!< エフェクト位置の基点
    float        buffEffectScale_      = 1.0f;                  //!< バフ上昇時エフェクトのスケール（JSONで設定）
    float        debuffEffectScale_    = 1.0f;                  //!< デバフ下降時エフェクトのスケール（JSONで設定）
    Vector3      buffEffectOffset_     = Vector3::Zero;          //!< バフエフェクトのプレイヤー位置からのオフセット
    Vector3      debuffEffectOffset_   = Vector3::Zero;          //!< デバフエフェクトのプレイヤー位置からのオフセット
    EffectHandle currentEffectHandle_  = INVALID_EFFECT_HANDLE; //!< 再生中エフェクトのハンドル（追従用）
    int          effectFrameTimer_     = 0;                     //!< エフェクト開始からの経過フレーム数
    // K2Engine は非ループエフェクト終了時に EffectEmitter を DeleteGO するため、
    // それ以降に SetEffectPosition を呼ぶとダングリングポインタ参照になりクラッシュする。
    // このフレーム数を超えたらハンドルを破棄して追従を止める（エフェクト時間より長めに設定）。
    static constexpr int kEffectFollowMaxFrames = 180; //!< 追従上限フレーム数（約3秒 @60fps）

public:
    EmotionEffectObserver()  = default;
    ~EmotionEffectObserver() = default;

    /** 初期化。BattleManager の Playing 開始時に呼ぶ */
    void Init(Player* player, EmotionSystem* emotionSystem);

    /** 毎フレーム呼ぶ。再生中エフェクトをプレイヤーに追従させる */
    void Update();

    /** 感情レベルが変化したときに呼ばれる（IEmotionObserver の実装） */
    void OnEmotionChanged(int oldLevel, int newLevel) override;
};
