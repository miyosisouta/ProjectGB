#pragma once
#include "IStatusModifier.h"
#include <vector>

class IEmotionObserver;

// 感情の7段階（Debuff3=-3 〜 Normal=0 〜 Buff3=3）
// int としてそのまま計算できるよう値を段階番号と対応させている
enum class EmotionLevel : int
{
    Debuff3 = -3,
    Debuff2 = -2,
    Debuff1 = -1,
    Normal  =  0,
    Buff1   =  1,
    Buff2   =  2,
    Buff3   =  3,
};

// 各感情段階のステータス倍率
struct EmotionModifier
{
    float attackMul = 1.0f;  //!< 攻撃力倍率
    float speedMul  = 1.0f;  //!< 移動速度倍率
};

// 感情システム
// プレイヤーの感情段階を管理し、ステータス倍率を IStatusModifier 経由で提供する
// ジャスト回避 / 強攻撃被弾 / HP閾値の3ルートで段階が変化する
class EmotionSystem : public IStatusModifier
{
public:
    static constexpr int LevelMin = -3;  //!< 下限（Debuff3）
    static constexpr int LevelMax =  3;  //!< 上限（Buff3）

private:
    EmotionLevel    currentLevel_ = EmotionLevel::Normal;  //!< 現在の感情レベル
    EmotionModifier modifierTable_[7];                     //!< 倍率テーブル。インデックス = static_cast<int>(level) + 3
    std::vector<IEmotionObserver*> observers_;             //!< UI・エフェクトへの通知リスト（Observerパターン）

public:
    EmotionSystem();
    ~EmotionSystem() = default;

    /** ParameterManager から倍率テーブルを上書きロードする。PlayerStatus::Init() から呼ぶ */
    void Init();

    /** ジャスト回避成功時に呼ぶ。デバフ中→Normal帳消し、通常/バフ中→1段階上昇 */
    void OnJustAvoid();

    /** 強攻撃ヒット時に呼ぶ（HitStamp・Spin・チャージレーザー）。バフ中→Normal帳消し、通常/デバフ中→1段階下降 */
    void OnStrongAttackHit();

    /** HP閾値などで強制的にレベルをセットする（GamePhaseManager から呼ぶ） */
    void ForceSetLevel(EmotionLevel level);

    /** 現在の感情レベルを取得する（UI・デバッグ用） */
    EmotionLevel GetCurrentLevel() const { return currentLevel_; }

    /** Observer を登録する（UIやエフェクトシステムから呼ぶ） */
    void AddObserver(IEmotionObserver* observer);

    /** Observer を解除する */
    void RemoveObserver(IEmotionObserver* observer);

    /************************************************/
    /** IStatusModifier の実装                      */
    /************************************************/
    // PlayerStatus の GetAttack() / GetMoveSpeedBase() から参照して倍率を適用する
    float GetAttackMul() const override;
    float GetSpeedMul()  const override;

private:
    /** レベルを変更し、値が変わった場合のみ全 Observer に通知する */
    void ChangeLevel(EmotionLevel newLevel);

    /** コンストラクタで呼ぶ。各段階の倍率テーブルを初期値で埋める */
    void InitModifierTable();
};
