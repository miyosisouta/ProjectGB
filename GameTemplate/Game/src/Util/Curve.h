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
    T m_startValue;          //!< 始める数値
    T m_endValue;            //!< 終わる数値
    float m_duration;        //!< 時間の間隔
    float m_currentTime;     //!< 現在の時間
    EasingType m_easingType; //!< イージングタイプ
    LoopMode m_loopMode;     //!< ループモード
    bool m_isPlaying;        //!< 再生するか
    int m_direction;         //!< 方向


public:
    Curve()
        : m_currentTime(0)
        , m_duration(1.0f)
        , m_isPlaying(false)
        , m_direction(1)
    {
    }

    /* 初期化 */
    void Initialize(const T& start, const T& end, const float timeSec, const EasingType type = EasingType::EaseInOut, const LoopMode loopMode = LoopMode::Once) {
        m_startValue = start;
        m_endValue = end;
        m_duration = max(0.0001f, timeSec);
        m_easingType = type;
        m_loopMode = loopMode;
        m_currentTime = 0.0f;
        m_isPlaying = false;
        m_direction = 1;
    }

    /** 再生 */
    void Play()
    {
        m_isPlaying = true;
    }

    /* 停止 */
    void Stop() {
        m_isPlaying = false;
    }

    /** リセット */
    void Reset() {
        m_currentTime = 0.0f;
        m_direction = 1;
    }

    /** 更新 */
    void Update(float deltaTime)
    {
        // 再生していなければ何もしない
        if (!m_isPlaying) return;

        // 時間を進める
        //m_currentTime += m_loopMode == LoopMode::Loop ? deltaTime * m_direction : deltaTime;
        if (m_loopMode == LoopMode::Loop)
        {
            m_currentTime += deltaTime * m_direction;
        }
        else if (m_loopMode == LoopMode::PingPong) {
            m_currentTime += deltaTime * m_direction;
        }
        else {
            m_currentTime += deltaTime;
        }

        // 終了判定とループ処理
        if (m_currentTime >= m_duration) {
            if (m_loopMode == LoopMode::Once) {
                m_currentTime = m_duration;
                m_isPlaying = false;
            }
            else if (m_loopMode == LoopMode::Loop) {
                m_currentTime = 0.0f;
            }
            else if (m_loopMode == LoopMode::PingPong) {
                m_currentTime = m_duration;
                m_direction = -1;
            }
        }
        else if (m_currentTime <= 0.0f) {
            if (m_loopMode == LoopMode::PingPong) {
                m_currentTime = 0.0f;
                m_direction = 1;
            }
        }
    }


    /** 現在の値を取得 */
    T GetCurrentValue() const
    {
        float t = clamp<float>(m_currentTime / m_duration, 0.0f, 1.0f);
        float rate = ApplyEasingInternal(t);
        return m_startValue + (m_endValue - m_startValue) * rate;
    }

    /** 再生中か取得 */
    bool IsPlaying() const { return m_isPlaying; }


private:
    /** イージング適用 */
    float ApplyEasingInternal(float t) const {
        switch (m_easingType) {
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
