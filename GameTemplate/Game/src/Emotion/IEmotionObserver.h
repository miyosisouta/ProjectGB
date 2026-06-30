#pragma once

// 感情レベルの変化をUIやエフェクトに通知するObserverインターフェース
// EmotionSystem はこのインターフェースを通じて通知するので UI 実装クラスを直接知らなくて済む
// UI側は AddObserver() で自分を登録し、OnEmotionChanged() を受け取るだけでよい
class IEmotionObserver
{
public:
    virtual ~IEmotionObserver() = default;

    /** 感情レベルが変化したときに呼ばれる。oldLevel/newLevel は -3〜3 の整数値 */
    // 両方渡すことでUI側で「上がった/下がった」「何段階目か」を判断できる
    virtual void OnEmotionChanged(int oldLevel, int newLevel) = 0;
};
