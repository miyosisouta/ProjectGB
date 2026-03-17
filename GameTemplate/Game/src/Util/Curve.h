#include "../k2EngineLow/math/Vector.h" // 追加: Vector2, Vector3, Vector4型の定義が必要

#pragma once

// 汎用的なclamp関数テンプレート
template <typename T>
T clamp(T value, T low, T high) {
    if (value < low) {
        return low;
    }
    if (value > high) {
        return high;
    }
    return value;
}

/**
 * @brief イージングとループの種類
 */

 /** イージングの種類 */
enum class EasingType { Linear, EaseIn, EaseOut, EaseInOut }; // 線形補間、イーズイン、イーズアウト、イーズインアウト
/** ループの種類 */
enum class LoopMode { Once, Loop, PingPong }; // 片道、周回(上から下、上から下を繰り返す)、往復(上から下、下から上を繰り返す)





/** 汎用的なカーブクラステンプレート */
template <typename T>
class Curve
{
public:
    T startValue;          //!< 始める数値
    T endValue;            //!< 終わる数値
    float duration;        //!< 時間の間隔
    float currentTime;     //!< 現在の時間
    EasingType easingType; //!< イージングタイプ
    LoopMode loopMode;     //!< ループモード
    bool isPlaying;        //!< 再生するか
    int direction;         //!< 方向


public:
    Curve()
        : currentTime(0)
        , duration(1.0f)
        , isPlaying(false)
        , direction(1)
    {
    }

    /* 初期化 */
    void Initialize(const T& start, const T& end, const float timeSec, const EasingType type = EasingType::EaseInOut, const LoopMode loopMode = LoopMode::Once) {
        startValue = start;
        endValue = end;
        duration = max(0.0001f, timeSec);
        easingType = type;
        loopMode = loopMode;
        currentTime = 0.0f;
        isPlaying = false;
        direction = 1;
    }

    /** 再生 */
    void Play()
    {
        isPlaying = true;
    }

    /* 停止 */
    void Stop() {
        isPlaying = false;
    }

    /** リセット */
    void Reset() {
        currentTime = 0.0f;
        direction = 1;
    }

    /** 更新 */
    void Update(float deltaTime)
    {
        // 再生していなければ何もしない
        if (!isPlaying) return;

        // 時間を進める
        //m_currentTime += m_loopMode == LoopMode::Loop ? deltaTime * m_direction : deltaTime;
        if (loopMode == LoopMode::Loop)
        {
            currentTime += deltaTime * m_direction;
        }
        else if (loopMode == LoopMode::PingPong) {
            currentTime += deltaTime * direction;
        }
        else {
            currentTime += deltaTime;
        }

        // 終了判定とループ処理
        if (currentTime >= duration) {
            if (loopMode == LoopMode::Once) {
                currentTime = duration;
                isPlaying = false;
            }
            else if (loopMode == LoopMode::Loop) {
                currentTime = 0.0f;
            }
            else if (loopMode == LoopMode::PingPong) {
                currentTime = duration;
                direction = -1;
            }
        }
        else if (currentTime <= 0.0f) {
            if (loopMode == LoopMode::PingPong) {
                currentTime = 0.0f;
                direction = 1;
            }
        }
    }


    /** 現在の値を取得 */
    T GetCurrentValue() const
    {
        float t = clamp<float>(currentTime / duration, 0.0f, 1.0f);
        float rate = ApplyEasingInternal(t);
        return startValue + (endValue - startValue) * rate;
    }

    /** 再生中か取得 */
    bool IsPlaying() const { return isPlaying; }


private:
    /** イージング適用 */
    float ApplyEasingInternal(float t) const {
        switch (easingType) {
        case EasingType::Linear:    return t;
        case EasingType::EaseIn:    return t * t;
        case EasingType::EaseOut:   return t * (2.0f - t);
        case EasingType::EaseInOut:
            if (t < 0.5f) return 2.0f * t * t;
            else          return -1.0f + (4.0f - 2.0f * t) * t;
        }
        return t;
    }
};

using FloatCurve = Curve<float>;
using Vector2Curve = Curve<Vector2>;
using Vector3Curve = Curve<Vector3>;
using Vector4Curve = Curve<Vector4>;
