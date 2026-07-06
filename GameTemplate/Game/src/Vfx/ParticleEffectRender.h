/**
 * ParticleEffectRender.h
 * パーティクルエフェクトの描画クラス
 * SpriteRenderを使って2Dパーティクルを描画する
 *
 * ホットリロード対応:
 *   - ファイルタイムスタンプ監視（ポーリング）
 *   - 手動リロード関数
 */
#pragma once
#include "ParticleEmitter.h"
#include "ParticleEffectLoader.h"
#include <vector>
#include <string>
#include <cmath>
#include <sys/stat.h>

/**
 * パーティクルエフェクトレンダラー
 * エミッター + スプライト描画を統合する
 */
class ParticleEffectRender
{
private:
    /** エミッター */
    ParticleEmitter emitter_;

    /** スプライトプール */
    std::vector<std::unique_ptr<SpriteRender>> sprites_;

    /** テクスチャ情報 */
    std::string texturePath_;
    float spriteWidth_;
    float spriteHeight_;

    /** 初期化済みか */
    bool isInitialized_;

    /** ホットリロード用 */
    std::string jsonFilePath_;
    bool hotReloadEnabled_;
    float hotReloadInterval_;     // チェック間隔（秒）
    float hotReloadTimer_;        // タイマー
    long long lastModifiedTime_;  // ファイル最終更新時刻

public:
    ParticleEffectRender()
        : spriteWidth_(64.0f)
        , spriteHeight_(64.0f)
        , isInitialized_(false)
        , hotReloadEnabled_(false)
        , hotReloadInterval_(1.0f)
        , hotReloadTimer_(0.0f)
        , lastModifiedTime_(0)
    {
    }

    /**
     * JSONファイルから初期化
     * @param jsonFilePath  エフェクト定義JSON
     * @param texFilePath   パーティクル用テクスチャ(.dds)
     * @param width         スプライトの基本幅
     * @param height        スプライトの基本高さ
     */
    void Init(
        const char* jsonFilePath,
        const char* texFilePath,
        float width = 64.0f,
        float height = 64.0f)
    {
        jsonFilePath_ = jsonFilePath;
        texturePath_ = texFilePath;
        spriteWidth_ = width;
        spriteHeight_ = height;

        // JSONからエミッターを構築
        ParticleEffectLoader::LoadFromFile(jsonFilePath, emitter_);

        // スプライトプール初期化
        InitSpritePool();

        // ファイル更新時刻を記録
        lastModifiedTime_ = GetFileModifiedTime(jsonFilePath_);

        isInitialized_ = true;
    }

    /** エミッターへの直接アクセス */
    ParticleEmitter& GetEmitter() { return emitter_; }

    // ----- ホットリロード設定 -----

    /**
     * ホットリロードを有効化
     * @param intervalSec  ファイル変更チェック間隔（秒）
     */
    void EnableHotReload(float intervalSec = 1.0f)
    {
        hotReloadEnabled_ = true;
        hotReloadInterval_ = intervalSec;
        hotReloadTimer_ = 0.0f;
    }

    /** ホットリロードを無効化 */
    void DisableHotReload()
    {
        hotReloadEnabled_ = false;
    }

    /**
     * 手動でJSONをリロード
     * 座標とプレイ状態は維持される
     */
    bool Reload()
    {
        if (jsonFilePath_.empty()) return false;

        // 現在の状態を保存
        Vector3 pos = emitter_.GetPosition();
        bool wasPlaying = emitter_.IsPlaying();

        // エミッターをクリアして再構築
        emitter_.ClearModules();
        bool success = ParticleEffectLoader::LoadFromFile(jsonFilePath_.c_str(), emitter_);

        if (success) {
            // スプライトプールのサイズが変わった場合は再初期化
            int maxParticles = emitter_.GetSpawnModule().GetMaxParticles();
            if (static_cast<int>(sprites_.size()) != maxParticles) {
                InitSpritePool();
            }

            // 状態復帰
            emitter_.SetPosition(pos);
            emitter_.Reset();
            if (wasPlaying) {
                emitter_.Play();
            }

            // 更新時刻を記録
            lastModifiedTime_ = GetFileModifiedTime(jsonFilePath_);
        }

        return success;
    }

    // ----- 制御 -----

    void Play()
    {
        // いつ読んでも最初から再生されるようにした
        Reset();
        emitter_.Play();
    }

    void Stop()
    {
        emitter_.Stop();
    }

    void Reset()
    {
        emitter_.Reset();
        for (size_t i = 0; i < sprites_.size(); ++i) {
            sprites_[i]->SetScale(Vector3::Zero);
        }
    }

    void SetPosition(const Vector3& pos)
    {
        emitter_.SetPosition(pos);
    }

    bool IsPlaying() const
    {
        return emitter_.IsPlaying();
    }

    // ----- 更新・描画 -----

    void Update(float deltaTime)
    {
        if (!isInitialized_) return;

        // ホットリロードチェック
        if (hotReloadEnabled_) {
            hotReloadTimer_ += deltaTime;
            if (hotReloadTimer_ >= hotReloadInterval_) {
                hotReloadTimer_ = 0.0f;
                CheckAndReload();
            }
        }

        // エミッター更新
        emitter_.Update(deltaTime);

        // パーティクル → スプライト反映
        const std::vector<Particle>& particles = emitter_.GetParticles();
        for (size_t i = 0; i < particles.size() && i < sprites_.size(); ++i) {
            const Particle& p = particles[i];

            if (!p.isAlive) {
                sprites_[i]->SetScale(Vector3::Zero);
                continue;
            }

            sprites_[i]->SetPosition(p.position);
            sprites_[i]->SetScale(p.scaleValue);

            Quaternion rot;
            float rad = p.rotationAngle * 3.14159265f / 180.0f;
            rot.SetRotation(Vector3(0.0f, 0.0f, 1.0f), rad);
            sprites_[i]->SetRotation(rot);

            sprites_[i]->SetMulColor(p.color);
        }

        for (size_t i = 0; i < sprites_.size(); ++i) {
            sprites_[i]->Update();
        }
    }

    void Draw(RenderContext& rc)
    {
        if (!isInitialized_) return;

        const std::vector<Particle>& particles = emitter_.GetParticles();
        for (size_t i = 0; i < particles.size() && i < sprites_.size(); ++i) {
            if (particles[i].isAlive) {
                sprites_[i]->Draw(rc);
            }
        }
    }

private:
    /** スプライトプールを（再）初期化 */
    void InitSpritePool()
    {
        int maxParticles = emitter_.GetSpawnModule().GetMaxParticles();
        sprites_.clear();
        sprites_.resize(maxParticles);
        for (int i = 0; i < maxParticles; ++i) {
            sprites_[i] = std::make_unique<SpriteRender>();
            sprites_[i]->Init(texturePath_.c_str(), spriteWidth_, spriteHeight_, AlphaBlendMode_Trans);
            sprites_[i]->SetScale(Vector3::Zero);
        }
    }

    /** ファイルの最終更新時刻を取得 */
    static long long GetFileModifiedTime(const std::string& filePath)
    {
        struct stat fileStat;
        if (stat(filePath.c_str(), &fileStat) == 0) {
            return static_cast<long long>(fileStat.st_mtime);
        }
        return 0;
    }

    /** ファイルが変更されていたらリロード */
    void CheckAndReload()
    {
        long long currentTime = GetFileModifiedTime(jsonFilePath_);
        if (currentTime != 0 && currentTime != lastModifiedTime_) {
            Reload();
        }
    }
};