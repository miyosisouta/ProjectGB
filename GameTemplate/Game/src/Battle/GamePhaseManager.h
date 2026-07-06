#pragma once

class BossCharacter;
class EmotionSystem;
enum class EmotionLevel;

// ボスのHP閾値を監視し、ボス自身の感情（強気⇔動揺）を強制的に変化させる調整役（Mediatorパターン）
// 「HP何%でどうなる」というゲームルールをここに集約する
class GamePhaseManager
{
private:
    BossCharacter*  boss_          = nullptr;  //!< HP監視対象のボス
    EmotionSystem*  emotionSystem_ = nullptr;  //!< 強制変更先の感情システム

    bool phase50Triggered_ = false;  //!< HP50%閾値の発火済みフラグ（重複発火を防ぐ）
    bool phase25Triggered_ = false;  //!< HP25%閾値の発火済みフラグ

public:
    GamePhaseManager()  = default;
    ~GamePhaseManager() = default;

    /** ボスと感情システムを渡して初期化する（BattleManager の Playing 1フレーム目に呼ぶ） */
    void Init(BossCharacter* boss, EmotionSystem* emotionSystem);

    /** 毎フレーム呼ぶ。HP割合を見て閾値を超えたら一度だけ対応する処理を呼ぶ */
    void Update();

private:
    /** HP50%以下：後がないという焦りから全力を出す（強制Buff3） */
    void OnBossPhase50();

    /** HP25%以下：追い詰められて冷静さを欠き、隙ができる（強制Debuff3） */
    void OnBossPhase25();
};
