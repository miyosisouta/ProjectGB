/**
 * Particle.h
 * 個別パーティクルのデータ
 */
#pragma once

/**
 * 1つのパーティクルが持つランタイムデータ
 */
struct Particle
{
    /** 生存フラグ */
    bool isAlive;

    /** 寿命管理 */
    float lifetime;
    float age;

    /** トランスフォーム */
    Vector3 position;
    Vector3 velocity;
    Vector3 acceleration;

    Vector3 scaleValue;
    FloatCurve scaleCurveX;
    FloatCurve scaleCurveY;
    bool hasScaleCurve;

    float rotationAngle;       // Z軸回転角度（degree）
    float angularVelocity;     // 回転速度（degree/sec）
    FloatCurve rotationCurve;
    bool hasRotationCurve;

    /** カラー */
    Vector4 color;
    FloatCurve alphaCurve;
    bool hasAlphaCurve;

    /** 速度カーブ */
    FloatCurve speedCurveX;
    FloatCurve speedCurveY;
    bool hasSpeedCurve;

    Particle()
        : isAlive(false)
        , lifetime(1.0f)
        , age(0.0f)
        , position(Vector3::Zero)
        , velocity(Vector3::Zero)
        , acceleration(Vector3::Zero)
        , scaleValue(Vector3::One)
        , hasScaleCurve(false)
        , rotationAngle(0.0f)
        , angularVelocity(0.0f)
        , hasRotationCurve(false)
        , color(1.0f, 1.0f, 1.0f, 1.0f)
        , hasAlphaCurve(false)
        , hasSpeedCurve(false)
    {
    }

    /** 正規化した寿命割合 0.0～1.0 */
    float GetNormalizedAge() const
    {
        if (lifetime <= 0.0f) return 1.0f;
        return clamp<float>(age / lifetime, 0.0f, 1.0f);
    }
};
