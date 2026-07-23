#pragma once

/**
 * ボスの感情（Buff⇔Debuff）の変化をUIやエフェクトに通知するObserverインターフェース
 * EmotionSystem はこのインターフェースを通じて通知するので UI・エフェクトの実装クラスを直接知らなくて済む
 * 実装側は AddObserver() で自分を登録し、OnEmotionChanged() を受け取るだけでよい
 */
class IEmotionObserver
{
public:
    /** デストラクタ */
    virtual ~IEmotionObserver() = default;

    /**
     * 感情のレベルが変化したときに呼ばれる。oldLevel/newLevel は -3〜3 の整数値
     * 両方渡すことで「BuffになったかDebuffになったか」「何段階目か」を判断できる
     */
    virtual void OnEmotionChanged(int oldLevel, int newLevel) = 0;
};
