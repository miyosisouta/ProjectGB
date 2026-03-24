/**
 * ParticleEmitter.h
 * パーティクルエミッター
 * モジュールを組み合わせてエフェクトを制御する
 */
#pragma once
#include "Particle.h"
#include "ParticleModule.h"
#include <vector>
#include <random>
#include <memory>

 /**
  * パーティクルエミッター
  * ナイアガラ風のモジュール式2Dエフェクトシステム
  */
class ParticleEmitter
{
private:
    /** パーティクルプール */
    std::vector<Particle> particles_;

    /** モジュール */
    SpawnModule spawnModule_;
    std::vector<ParticleModule*> initModules_;
    std::vector<ParticleModule*> updateModules_;

    /** メモリ管理 */
    std::vector<std::unique_ptr<ParticleModule>> ownedModules_;

    /** 乱数 */
    std::mt19937 rng_;

    /** エミッター状態 */
    bool isPlaying_;
    float emitterAge_;
    float emitterDuration_;  // 0以下で無限
    int aliveCount_;

    /** エミッター座標 */
    Vector3 emitterPosition_;

public:
    ParticleEmitter()
        : isPlaying_(false)
        , emitterAge_(0.0f)
        , emitterDuration_(0.0f)
        , aliveCount_(0)
        , emitterPosition_(Vector3::Zero)
    {
        std::random_device rd;
        rng_.seed(rd());
        particles_.resize(spawnModule_.GetMaxParticles());
    }

    ~ParticleEmitter()
    {
        initModules_.clear();
        updateModules_.clear();
        ownedModules_.clear();
    }

    /**
     * 全モジュールをクリアする（ホットリロード用）
     * SpawnModuleはデフォルトにリセットされる
     */
    void ClearModules()
    {
        initModules_.clear();
        updateModules_.clear();
        ownedModules_.clear();
        spawnModule_ = SpawnModule();
    }

    // ----- モジュール追加 -----

    SpawnModule& GetSpawnModule() { return spawnModule_; }

    template <typename T>
    T* AddModule()
    {
        std::unique_ptr<T> mod(new T());
        T* ptr = mod.get();

        ParticleModuleType type = ptr->GetType();
        switch (type)
        {
        case ParticleModuleType::InitLifetime:
        case ParticleModuleType::InitPosition:
        case ParticleModuleType::InitVelocity:
        case ParticleModuleType::InitScale:
        case ParticleModuleType::InitRotation:
        case ParticleModuleType::InitColor:
            initModules_.push_back(ptr);
            break;

        case ParticleModuleType::ScaleOverLife:
        case ParticleModuleType::RotationOverLife:
        case ParticleModuleType::AlphaOverLife:
        case ParticleModuleType::SpeedOverLife:
            initModules_.push_back(ptr);
            updateModules_.push_back(ptr);
            break;

        case ParticleModuleType::Acceleration:
        case ParticleModuleType::VelocityDamping:
            updateModules_.push_back(ptr);
            break;

        default:
            break;
        }

        ownedModules_.push_back(std::move(mod));
        return ptr;
    }

    // ----- 制御 -----

    void Play()
    {
        isPlaying_ = true;
        emitterAge_ = 0.0f;
        spawnModule_.Reset();
    }

    void Stop() { isPlaying_ = false; }

    void Reset()
    {
        isPlaying_ = false;
        emitterAge_ = 0.0f;
        aliveCount_ = 0;
        spawnModule_.Reset();
        for (size_t i = 0; i < particles_.size(); ++i) {
            particles_[i].isAlive = false;
        }
    }

    void SetPosition(const Vector3& pos) { emitterPosition_ = pos; }
    const Vector3& GetPosition() const { return emitterPosition_; }
    void SetDuration(float duration) { emitterDuration_ = duration; }
    bool IsPlaying() const { return isPlaying_; }
    int GetAliveCount() const { return aliveCount_; }

    // ----- 更新 -----

    void Update(float deltaTime)
    {
        if (!isPlaying_) return;

        emitterAge_ += deltaTime;
        bool canSpawn = true;
        if (emitterDuration_ > 0.0f && emitterAge_ >= emitterDuration_) {
            canSpawn = false;
            if (aliveCount_ == 0) {
                isPlaying_ = false;
                return;
            }
        }

        // 生成
        if (canSpawn) {
            int count = spawnModule_.CalcSpawnCount(deltaTime, aliveCount_);
            for (int i = 0; i < count; ++i) {
                SpawnParticle();
            }
        }

        // 更新
        aliveCount_ = 0;
        for (size_t i = 0; i < particles_.size(); ++i) {
            Particle& p = particles_[i];
            if (!p.isAlive) continue;

            p.age += deltaTime;
            if (p.age >= p.lifetime) {
                p.isAlive = false;
                continue;
            }

            for (size_t m = 0; m < updateModules_.size(); ++m) {
                if (updateModules_[m]->IsEnabled()) {
                    updateModules_[m]->OnParticleUpdate(p, deltaTime);
                }
            }

            if (!p.hasRotationCurve) {
                p.rotationAngle += p.angularVelocity * deltaTime;
            }

            p.position.x += p.velocity.x * deltaTime;
            p.position.y += p.velocity.y * deltaTime;
            p.position.z += p.velocity.z * deltaTime;

            aliveCount_++;
        }
    }

    const std::vector<Particle>& GetParticles() const { return particles_; }

    void ResizePool()
    {
        particles_.resize(spawnModule_.GetMaxParticles());
    }

private:
    void SpawnParticle()
    {
        for (size_t i = 0; i < particles_.size(); ++i) {
            if (!particles_[i].isAlive) {
                Particle& p = particles_[i];
                p = Particle();
                p.isAlive = true;
                p.position = Vector3::Zero;

                for (size_t m = 0; m < initModules_.size(); ++m) {
                    if (initModules_[m]->IsEnabled()) {
                        initModules_[m]->OnParticleSpawn(p, rng_);
                    }
                }

                // InitPositionはローカル座標 → エミッター座標を加算
                p.position.x += emitterPosition_.x;
                p.position.y += emitterPosition_.y;
                p.position.z += emitterPosition_.z;

                return;
            }
        }
    }
};