/**
 * ParticleModule.h
 * パーティクルモジュール群
 * ナイアガラ風のモジュール式エフェクトシステム
 */
#pragma once
#include "Particle.h"
#include "ParticleValueProvider.h"
#include <vector>
#include <random>
#include <string>


 // -------------------------------------------------------
 // モジュール基底クラス
 // -------------------------------------------------------

 /**
  * モジュールの種類
  */
enum class ParticleModuleType
{
    Spawn,          // 生成
    InitLifetime,   // 寿命初期化
    InitPosition,   // 初期座標
    InitVelocity,   // 初期速度
    InitScale,      // 初期スケール
    InitRotation,   // 初期回転
    InitColor,      // 初期カラー
    ScaleOverLife,  // 寿命に応じたスケール変化
    RotationOverLife,// 寿命に応じた回転変化
    AlphaOverLife,  // 寿命に応じた透明度変化
    SpeedOverLife,  // 寿命に応じた速度変化
    Acceleration,   // 加速度（重力等）
    VelocityDamping // 速度減衰
};


/**
 * パーティクルモジュール基底
 */
class ParticleModule
{
protected:
    ParticleModuleType type_;
    bool enabled_;

public:
    ParticleModule(ParticleModuleType type)
        : type_(type), enabled_(true)
    {
    }

    virtual ~ParticleModule() {}

    ParticleModuleType GetType() const { return type_; }
    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

    /** パーティクル生成時に呼ばれる */
    virtual void OnParticleSpawn(Particle& p, std::mt19937& rng) {}

    /** 毎フレーム呼ばれる */
    virtual void OnParticleUpdate(Particle& p, float deltaTime) {}
};


// -------------------------------------------------------
// スポーンモジュール
// -------------------------------------------------------

/**
 * パーティクルの生成制御
 */
class SpawnModule : public ParticleModule
{
private:
    float spawnRate_;          // 1秒あたりの生成数
    int burstCount_;           // 一括生成数
    float burstInterval_;      // バースト間隔（秒）
    int maxParticles_;         // 最大パーティクル数
    bool isLooping_;           // ループするか

    // ランタイム
    float spawnAccumulator_;
    float burstTimer_;
    int burstsFired_;

public:
    SpawnModule()
        : ParticleModule(ParticleModuleType::Spawn)
        , spawnRate_(10.0f)
        , burstCount_(0)
        , burstInterval_(0.0f)
        , maxParticles_(100)
        , isLooping_(true)
        , spawnAccumulator_(0.0f)
        , burstTimer_(0.0f)
        , burstsFired_(0)
    {
    }

    void SetSpawnRate(float rate) { spawnRate_ = rate; }
    void SetBurst(int count, float interval = 0.0f) { burstCount_ = count; burstInterval_ = interval; }
    void SetMaxParticles(int maxCount) { maxParticles_ = maxCount; }
    void SetLooping(bool loop) { isLooping_ = loop; }

    float GetSpawnRate() const { return spawnRate_; }
    int GetBurstCount() const { return burstCount_; }
    float GetBurstInterval() const { return burstInterval_; }
    int GetMaxParticles() const { return maxParticles_; }
    bool IsLooping() const { return isLooping_; }

    /**
     * 今フレームで生成すべきパーティクル数を計算
     */
    int CalcSpawnCount(float deltaTime, int currentAliveCount)
    {
        int spawnCount = 0;

        // レート生成
        if (spawnRate_ > 0.0f) {
            spawnAccumulator_ += spawnRate_ * deltaTime;
            int rateCount = static_cast<int>(spawnAccumulator_);
            spawnAccumulator_ -= rateCount;
            spawnCount += rateCount;
        }

        // バースト生成
        if (burstCount_ > 0) {
            burstTimer_ += deltaTime;
            if (burstsFired_ == 0 || (burstInterval_ > 0.0f && burstTimer_ >= burstInterval_)) {
                spawnCount += burstCount_;
                burstTimer_ = 0.0f;
                if (!isLooping_) {
                    burstsFired_++;
                }
            }
        }

        // 最大数制限
        int remaining = maxParticles_ - currentAliveCount;
        if (remaining < 0) remaining = 0;
        if (spawnCount > remaining) spawnCount = remaining;

        return spawnCount;
    }

    void Reset()
    {
        spawnAccumulator_ = 0.0f;
        burstTimer_ = 0.0f;
        burstsFired_ = 0;
    }
};


// -------------------------------------------------------
// 初期化モジュール群
// -------------------------------------------------------

/** 寿命初期化 */
class InitLifetimeModule : public ParticleModule
{
private:
    FloatValueProvider lifetime_;

public:
    InitLifetimeModule()
        : ParticleModule(ParticleModuleType::InitLifetime)
    {
        lifetime_.SetFixed(1.0f);
    }

    FloatValueProvider& Lifetime() { return lifetime_; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        p.lifetime = lifetime_.ResolveInitial(rng);
        p.age = 0.0f;
    }
};


/** 初期座標 */
class InitPositionModule : public ParticleModule
{
private:
    Vector3ValueProvider position_;

public:
    InitPositionModule()
        : ParticleModule(ParticleModuleType::InitPosition)
    {
    }

    Vector3ValueProvider& Position() { return position_; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        p.position = position_.ResolveInitial(rng);
    }
};


/** 初期速度 */
class InitVelocityModule : public ParticleModule
{
private:
    Vector3ValueProvider velocity_;

public:
    InitVelocityModule()
        : ParticleModule(ParticleModuleType::InitVelocity)
    {
    }

    Vector3ValueProvider& Velocity() { return velocity_; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        p.velocity = velocity_.ResolveInitial(rng);
    }
};


/** 初期スケール */
class InitScaleModule : public ParticleModule
{
private:
    Vector3ValueProvider scale_;

public:
    InitScaleModule()
        : ParticleModule(ParticleModuleType::InitScale)
    {
    }

    Vector3ValueProvider& Scale() { return scale_; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        p.scaleValue = scale_.ResolveInitial(rng);
    }
};


/** 初期回転（Z軸） */
class InitRotationModule : public ParticleModule
{
private:
    FloatValueProvider angle_;           // 初期角度
    FloatValueProvider angularVelocity_; // 回転速度

public:
    InitRotationModule()
        : ParticleModule(ParticleModuleType::InitRotation)
    {
        angle_.SetFixed(0.0f);
        angularVelocity_.SetFixed(0.0f);
    }

    FloatValueProvider& Angle() { return angle_; }
    FloatValueProvider& AngularVelocity() { return angularVelocity_; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        p.rotationAngle = angle_.ResolveInitial(rng);
        p.angularVelocity = angularVelocity_.ResolveInitial(rng);
    }
};


/** 初期カラー */
class InitColorModule : public ParticleModule
{
private:
    FloatValueProvider r_;
    FloatValueProvider g_;
    FloatValueProvider b_;
    FloatValueProvider a_;

public:
    InitColorModule()
        : ParticleModule(ParticleModuleType::InitColor)
    {
        r_.SetFixed(1.0f);
        g_.SetFixed(1.0f);
        b_.SetFixed(1.0f);
        a_.SetFixed(1.0f);
    }

    FloatValueProvider& R() { return r_; }
    FloatValueProvider& G() { return g_; }
    FloatValueProvider& B() { return b_; }
    FloatValueProvider& A() { return a_; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        p.color.x = r_.ResolveInitial(rng);
        p.color.y = g_.ResolveInitial(rng);
        p.color.z = b_.ResolveInitial(rng);
        p.color.w = a_.ResolveInitial(rng);
    }
};


// -------------------------------------------------------
// OverLifetimeモジュール群
// -------------------------------------------------------

/** 寿命に応じたスケール変化 */
class ScaleOverLifeModule : public ParticleModule
{
private:
    FloatValueProvider startScaleX_;
    FloatValueProvider endScaleX_;
    FloatValueProvider startScaleY_;
    FloatValueProvider endScaleY_;
    EasingType easingType_;
    bool uniform_;  // true: XY同値（均等スケール）

public:
    ScaleOverLifeModule()
        : ParticleModule(ParticleModuleType::ScaleOverLife)
        , easingType_(EasingType::Linear)
        , uniform_(false)
    {
        startScaleX_.SetFixed(1.0f);
        endScaleX_.SetFixed(0.0f);
        startScaleY_.SetFixed(1.0f);
        endScaleY_.SetFixed(0.0f);
    }

    FloatValueProvider& StartScaleX() { return startScaleX_; }
    FloatValueProvider& EndScaleX() { return endScaleX_; }
    FloatValueProvider& StartScaleY() { return startScaleY_; }
    FloatValueProvider& EndScaleY() { return endScaleY_; }
    void SetEasing(EasingType type) { easingType_ = type; }
    void SetUniform(bool uniform) { uniform_ = uniform; }
    bool IsUniform() const { return uniform_; }

    /**
     * uniform=true のとき、startScaleX/endScaleX を XY共通値として使う。
     * SetUniformScale() で一括設定できる。
     */
    void SetUniformScale(const FloatValueProvider& start, const FloatValueProvider& end)
    {
        uniform_ = true;
        startScaleX_ = start;
        endScaleX_ = end;
    }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        float sx, ex, sy, ey;

        if (uniform_) {
            // XY同値: Xのプロバイダから1回だけ値を取り、Yにも同じ値を使う
            sx = startScaleX_.ResolveInitial(rng);
            ex = endScaleX_.ResolveInitial(rng);
            sy = sx;
            ey = ex;
        }
        else {
            sx = startScaleX_.ResolveInitial(rng);
            ex = endScaleX_.ResolveInitial(rng);
            sy = startScaleY_.ResolveInitial(rng);
            ey = endScaleY_.ResolveInitial(rng);
        }

        p.scaleCurveX.Initialize(sx, ex, p.lifetime, easingType_, LoopMode::Once);
        p.scaleCurveY.Initialize(sy, ey, p.lifetime, easingType_, LoopMode::Once);
        p.scaleCurveX.Play();
        p.scaleCurveY.Play();
        p.hasScaleCurve = true;

        // 初期スケールにも適用
        p.scaleValue.x = sx;
        p.scaleValue.y = sy;
    }

    void OnParticleUpdate(Particle& p, float deltaTime) override
    {
        if (!p.hasScaleCurve) return;
        p.scaleCurveX.Update(deltaTime);
        p.scaleCurveY.Update(deltaTime);
        p.scaleValue.x = p.scaleCurveX.GetCurrentValue();
        p.scaleValue.y = p.scaleCurveY.GetCurrentValue();
    }
};


/** 寿命に応じた回転変化 */
class RotationOverLifeModule : public ParticleModule
{
private:
    FloatValueProvider startAngle_;
    FloatValueProvider endAngle_;
    EasingType easingType_;

public:
    RotationOverLifeModule()
        : ParticleModule(ParticleModuleType::RotationOverLife)
        , easingType_(EasingType::Linear)
    {
        startAngle_.SetFixed(0.0f);
        endAngle_.SetFixed(360.0f);
    }

    FloatValueProvider& StartAngle() { return startAngle_; }
    FloatValueProvider& EndAngle() { return endAngle_; }
    void SetEasing(EasingType type) { easingType_ = type; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        float s = startAngle_.ResolveInitial(rng);
        float e = endAngle_.ResolveInitial(rng);
        p.rotationCurve.Initialize(s, e, p.lifetime, easingType_, LoopMode::Once);
        p.rotationCurve.Play();
        p.hasRotationCurve = true;
        p.rotationAngle = s;
    }

    void OnParticleUpdate(Particle& p, float deltaTime) override
    {
        if (!p.hasRotationCurve) return;
        p.rotationCurve.Update(deltaTime);
        p.rotationAngle = p.rotationCurve.GetCurrentValue();
    }
};


/** 寿命に応じた透明度変化 */
class AlphaOverLifeModule : public ParticleModule
{
private:
    FloatValueProvider startAlpha_;
    FloatValueProvider endAlpha_;
    EasingType easingType_;

public:
    AlphaOverLifeModule()
        : ParticleModule(ParticleModuleType::AlphaOverLife)
        , easingType_(EasingType::Linear)
    {
        startAlpha_.SetFixed(1.0f);
        endAlpha_.SetFixed(0.0f);
    }

    FloatValueProvider& StartAlpha() { return startAlpha_; }
    FloatValueProvider& EndAlpha() { return endAlpha_; }
    void SetEasing(EasingType type) { easingType_ = type; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        float s = startAlpha_.ResolveInitial(rng);
        float e = endAlpha_.ResolveInitial(rng);
        p.alphaCurve.Initialize(s, e, p.lifetime, easingType_, LoopMode::Once);
        p.alphaCurve.Play();
        p.hasAlphaCurve = true;
        p.color.w = s;
    }

    void OnParticleUpdate(Particle& p, float deltaTime) override
    {
        if (!p.hasAlphaCurve) return;
        p.alphaCurve.Update(deltaTime);
        p.color.w = p.alphaCurve.GetCurrentValue();
    }
};


/** 寿命に応じた速度変化 */
class SpeedOverLifeModule : public ParticleModule
{
private:
    FloatValueProvider startMultiplier_;
    FloatValueProvider endMultiplier_;
    EasingType easingType_;

public:
    SpeedOverLifeModule()
        : ParticleModule(ParticleModuleType::SpeedOverLife)
        , easingType_(EasingType::Linear)
    {
        startMultiplier_.SetFixed(1.0f);
        endMultiplier_.SetFixed(0.0f);
    }

    FloatValueProvider& StartMultiplier() { return startMultiplier_; }
    FloatValueProvider& EndMultiplier() { return endMultiplier_; }
    void SetEasing(EasingType type) { easingType_ = type; }

    void OnParticleSpawn(Particle& p, std::mt19937& rng) override
    {
        float s = startMultiplier_.ResolveInitial(rng);
        float e = endMultiplier_.ResolveInitial(rng);
        p.speedCurveX.Initialize(s, e, p.lifetime, easingType_, LoopMode::Once);
        p.speedCurveY.Initialize(s, e, p.lifetime, easingType_, LoopMode::Once);
        p.speedCurveX.Play();
        p.speedCurveY.Play();
        p.hasSpeedCurve = true;
    }

    void OnParticleUpdate(Particle& p, float deltaTime) override
    {
        if (!p.hasSpeedCurve) return;
        p.speedCurveX.Update(deltaTime);
        p.speedCurveY.Update(deltaTime);
        // 速度にマルチプライヤを適用（初期速度の方向を維持）
        // ※ 速度自体は変更せず、位置計算時にマルチプライヤをかける方式も検討
    }
};


/** 加速度モジュール（重力等） */
class AccelerationModule : public ParticleModule
{
private:
    Vector3 acceleration_;

public:
    AccelerationModule()
        : ParticleModule(ParticleModuleType::Acceleration)
        , acceleration_(Vector3::Zero)
    {
    }

    void SetAcceleration(const Vector3& accel) { acceleration_ = accel; }
    const Vector3& GetAcceleration() const { return acceleration_; }

    void OnParticleUpdate(Particle& p, float deltaTime) override
    {
        p.velocity.x += acceleration_.x * deltaTime;
        p.velocity.y += acceleration_.y * deltaTime;
        p.velocity.z += acceleration_.z * deltaTime;
    }
};


/** 速度減衰モジュール */
class VelocityDampingModule : public ParticleModule
{
private:
    float damping_; // 0.0～1.0。1.0で即停止、0.0で減衰なし

public:
    VelocityDampingModule()
        : ParticleModule(ParticleModuleType::VelocityDamping)
        , damping_(0.0f)
    {
    }

    void SetDamping(float d) { damping_ = clamp<float>(d, 0.0f, 1.0f); }
    float GetDamping() const { return damping_; }

    void OnParticleUpdate(Particle& p, float deltaTime) override
    {
        float factor = 1.0f - damping_ * deltaTime;
        if (factor < 0.0f) factor = 0.0f;
        p.velocity.x *= factor;
        p.velocity.y *= factor;
        p.velocity.z *= factor;
    }
};