#pragma once
#include "IBossStatusModifier.h"
#include <vector>

class IEmotionObserver;

/**
 * ボスの感情の7段階（Debuff3=-3 〜 Normal=0 〜 Buff3=3）
 * Debuff側 = プレイヤーにジャスト回避されて焦る
 * Buff側   = プレイヤーに特定の攻撃を当てて調子に乗る
 * int としてそのまま計算できるよう値を段階番号と対応させている
 */
enum class EmotionLevel : int
{
    Debuff3 = -3, //!< Debuff最大
    Debuff2 = -2, //!< Debuff2段階目
    Debuff1 = -1, //!< Debuff1段階目
    Normal  =  0, //!< 通常
    Buff1   =  1, //!< Buff1段階目
    Buff2   =  2, //!< Buff2段階目
    Buff3   =  3, //!< Buff最大
};

/** 各段階のステータス倍率 */
struct EmotionModifier
{
    float attackMul          = 1.0f;  //!< 攻撃力倍率
    float attackSpeedMul     = 1.0f;  //!< 攻撃速度倍率（taskSchedulerの秒数・攻撃間隔に反映。演出速度とは独立）
    float damageTakenMul     = 1.0f;  //!< 被ダメージ倍率
    float animationSpeedMul  = 1.0f;  //!< アニメーション再生速度倍率（攻撃速度とは独立に調整できる演出用の値）
    float effectSpeedMul     = 1.0f;  //!< エフェクト再生速度倍率（攻撃速度とは独立に調整できる演出用の値）
};

/**
 * ボスの感情システム（コアシステム）
 * 旧来はプレイヤーにバフ・デバフを施していたが、今回はボス自身の感情（Buff⇔Debuff）として管理する。
 * 「見た目や行動そのものが変化した方が分かりやすい」という理由からボス側の状態として実装している。
 * BossStatus が IBossStatusModifier* のリストにこれを登録し、攻撃力・攻撃速度・被ダメージの倍率を提供する。
 * 各段階の倍率は EmotionParameter.json 側で自由に調整できる（InitModifierTable() の値は JSON 未読み込み時のフォールバック）。
 */
class EmotionSystem : public IBossStatusModifier
{
private:
    EmotionLevel    currentLevel_ = EmotionLevel::Normal;  //!< 現在の感情レベル
    EmotionModifier modifierTable_[7];                     //!< 倍率テーブル。インデックス = static_cast<int>(level) + 3
    std::vector<IEmotionObserver*> observers_;              //!< UI・エフェクトへの通知リスト（Observerパターン）


public:
    static constexpr int LevelMin = -3;  //!< 下限（Debuff3）
    static constexpr int LevelMax =  3;  //!< 上限（Buff3）


private:
    /** レベルを変更し、値が変わった場合のみ全 Observer に通知する */
    void ChangeLevel(EmotionLevel newLevel);

    /** コンストラクタで呼ぶ。各段階の倍率テーブルを初期値で埋める */
    void InitModifierTable();


public:
    /** コンストラクタ */
    EmotionSystem();
    /** デストラクタ */
    ~EmotionSystem() = default;

    /** ParameterManager から倍率テーブルを上書きロードする。BossStatus::Init() から呼ぶ */
    void Init();

    /** ボスの特定攻撃（HitStamp・チャージレーザー・SpinAttack・ThrowRock）がプレイヤーにヒットしたときに呼ぶ */
    /** Debuff中→1段階だけ回復する、通常/Buff中→1段階Buffが上がる（どちらも1段階ずつの変化、上限 Buff3 でクランプ） */
    void OnAttackHitPlayer();

    /** プレイヤーのジャスト回避が成立したときに呼ぶ */
    /** Buff中→調子の良さが帳消しになりNormal、通常/Debuff中→1段階Debuffが深まる */
    void OnJustAvoidedByPlayer();

    /** HP閾値などで強制的にレベルをセットする（BossEmotionPhaseManager から呼ぶ） */
    void ForceSetLevel(EmotionLevel level);

    /** 現在の感情レベルを取得する（UI・デバッグ用） */
    inline EmotionLevel GetCurrentLevel() const { return currentLevel_; }

    /** Observer を登録する（UIやエフェクトシステムから呼ぶ） */
    void AddObserver(IEmotionObserver* observer);

    /** Observer を解除する */
    void RemoveObserver(IEmotionObserver* observer);

    /************************************************/
    /** IBossStatusModifier の実装                   */
    /************************************************/
    // BossStatus の GetAttack() / GetAttackSpeedMul() / GetDamageTakenMul() / GetAnimationSpeedMul() / GetEffectSpeedMul() から参照して倍率を適用する
    /** 攻撃力倍率を取得 */
    float GetAttackMul()         const override;
    /** 攻撃速度倍率を取得 */
    float GetAttackSpeedMul()    const override;
    /** 被ダメージ倍率を取得 */
    float GetDamageTakenMul()    const override;
    /** アニメーション再生速度倍率を取得 */
    float GetAnimationSpeedMul() const override;
    /** エフェクト再生速度倍率を取得 */
    float GetEffectSpeedMul()    const override;
};
