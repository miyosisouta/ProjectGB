#pragma once

class BossCharacter;
class EmotionSystem;
enum class EmotionLevel;

/**
 * ボスのHP閾値を監視し、ボス自身の感情（Buff⇔Debuff）を強制的に変化させる調整役（Mediatorパターン）
 * 「HP何%でどうなる」という感情システム専用のゲームルールをここに集約する
 * NOTE: ポーズなど他のゲームフェーズ制御とは無関係。あくまで感情システム専用のクラス
 */
class BossEmotionPhaseManager
{
private:
    BossCharacter*  boss_          = nullptr;  //!< HP監視対象のボス
    EmotionSystem*  emotionSystem_ = nullptr;  //!< 強制変更先の感情システム

public:
    /** コンストラクタ */
    BossEmotionPhaseManager()  = default;
    /** デストラクタ */
    ~BossEmotionPhaseManager() = default;

    /** ボスと感情システムを渡して初期化する（BattleManager の Playing 1フレーム目に呼ぶ） */
    void Init(BossCharacter* boss, EmotionSystem* emotionSystem);

    /** 毎フレーム呼ぶ。現状HP閾値の自動発火は行わない（Buff/Debuffとも演出タイミングに同期させるため） */
    void Update();

    /**
     * HP50%以下：強制Buff3
     * NOTE: HP閾値を跨いだ瞬間ではなく、ボスHP閾値カットシーンの演出タイミング（BattleManager側）に
     *       同期させて呼ぶための公開メソッド。Update() 側では自動発火しない。
     */
    void ForceApplyPhase50Buff();

    /**
     * HP25%以下：強制Debuff3
     * NOTE: HP閾値を跨いだ瞬間ではなく、ボスHP閾値カットシーンの演出タイミング（BattleManager側）に
     *       同期させて呼ぶための公開メソッド。Update() 側では自動発火しない。
     */
    void ForceApplyPhase25Debuff();
};
