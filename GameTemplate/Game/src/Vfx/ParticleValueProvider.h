/**
 * ParticleValueProvider.h
 * パーティクルパラメータの値供給
 * 固定値、ランダム範囲、カーブ補間に対応
 */
#pragma once
#include <random>
#include <string>

 /**
  * 値の供給モード
  * Fixed:    固定値
  * Random:   最小～最大のランダム
  * Curve:    開始値→終了値をイージングで補間
  * RandomCurve: ランダムで決定した開始値→終了値をイージングで補間
  */
enum class ValueMode
{
    Fixed,
    Random,
    Curve,
    RandomCurve
};


/**
 * float用の値プロバイダ
 */
class FloatValueProvider
{
private:
    ValueMode mode_;
    float fixedValue_;
    float minValue_;
    float maxValue_;
    float startValue_;
    float endValue_;
    EasingType easingType_;

public:
    FloatValueProvider()
        : mode_(ValueMode::Fixed)
        , fixedValue_(0.0f)
        , minValue_(0.0f)
        , maxValue_(0.0f)
        , startValue_(0.0f)
        , endValue_(0.0f)
        , easingType_(EasingType::Linear)
    {
    }

    void SetFixed(float value)
    {
        mode_ = ValueMode::Fixed;
        fixedValue_ = value;
    }

    void SetRandom(float minVal, float maxVal)
    {
        mode_ = ValueMode::Random;
        minValue_ = minVal;
        maxValue_ = maxVal;
    }

    void SetCurve(float start, float end, EasingType easing = EasingType::Linear)
    {
        mode_ = ValueMode::Curve;
        startValue_ = start;
        endValue_ = end;
        easingType_ = easing;
    }

    void SetRandomCurve(float minStart, float maxStart, float minEnd, float maxEnd, EasingType easing = EasingType::Linear)
    {
        mode_ = ValueMode::RandomCurve;
        minValue_ = minStart;
        maxValue_ = maxStart;
        startValue_ = minEnd;
        endValue_ = maxEnd;
        easingType_ = easing;
    }

    ValueMode GetMode() const { return mode_; }
    EasingType GetEasingType() const { return easingType_; }
    float GetFixedValue() const { return fixedValue_; }
    float GetMinValue() const { return minValue_; }
    float GetMaxValue() const { return maxValue_; }
    float GetStartValue() const { return startValue_; }
    float GetEndValue() const { return endValue_; }

    /** 初期値を決定する（パーティクル生成時に呼ぶ） */
    float ResolveInitial(std::mt19937& rng) const
    {
        switch (mode_) {
        case ValueMode::Fixed:
            return fixedValue_;
        case ValueMode::Random:
        {
            std::uniform_real_distribution<float> dist(minValue_, maxValue_);
            return dist(rng);
        }
        case ValueMode::Curve:
            return startValue_;
        case ValueMode::RandomCurve:
        {
            std::uniform_real_distribution<float> dist(minValue_, maxValue_);
            return dist(rng);
        }
        }
        return fixedValue_;
    }

    /** カーブの終了値を決定する（RandomCurve用） */
    float ResolveEnd(std::mt19937& rng) const
    {
        if (mode_ == ValueMode::RandomCurve) {
            std::uniform_real_distribution<float> dist(startValue_, endValue_);
            return dist(rng);
        }
        return endValue_;
    }

    /** カーブ系かどうか */
    bool IsCurveMode() const
    {
        return mode_ == ValueMode::Curve || mode_ == ValueMode::RandomCurve;
    }
};


/**
 * Vector3用の値プロバイダ
 */
class Vector3ValueProvider
{
private:
    FloatValueProvider x_;
    FloatValueProvider y_;
    FloatValueProvider z_;
    bool uniform_;  // true: XY(Z)を同じランダム値にする

public:
    Vector3ValueProvider()
        : uniform_(false)
    {
    }

    FloatValueProvider& X() { return x_; }
    FloatValueProvider& Y() { return y_; }
    FloatValueProvider& Z() { return z_; }

    const FloatValueProvider& X() const { return x_; }
    const FloatValueProvider& Y() const { return y_; }
    const FloatValueProvider& Z() const { return z_; }

    void SetUniform(bool uniform) { uniform_ = uniform; }
    bool IsUniform() const { return uniform_; }

    void SetFixed(const Vector3& value)
    {
        x_.SetFixed(value.x);
        y_.SetFixed(value.y);
        z_.SetFixed(value.z);
    }

    void SetRandom(const Vector3& minVal, const Vector3& maxVal)
    {
        x_.SetRandom(minVal.x, maxVal.x);
        y_.SetRandom(minVal.y, maxVal.y);
        z_.SetRandom(minVal.z, maxVal.z);
    }

    void SetCurve(const Vector3& start, const Vector3& end, EasingType easing = EasingType::Linear)
    {
        x_.SetCurve(start.x, end.x, easing);
        y_.SetCurve(start.y, end.y, easing);
        z_.SetCurve(start.z, end.z, easing);
    }

    /**
     * uniform=true のとき、Xプロバイダの結果をYにもコピーする。
     * Zは独立（2Dなので通常0固定）。
     */
    Vector3 ResolveInitial(std::mt19937& rng) const
    {
        float xVal = x_.ResolveInitial(rng);
        float yVal;
        if (uniform_) {
            yVal = xVal;  // Xと同じ値
        }
        else {
            yVal = y_.ResolveInitial(rng);
        }
        float zVal = z_.ResolveInitial(rng);
        return Vector3(xVal, yVal, zVal);
    }
};