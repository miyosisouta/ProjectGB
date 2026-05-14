#pragma once
/**
 * GrassBendManager.h
 *
 * 草の頂点曲げパラメータをGPUへ送るシングルトンマネージャー。
 * 攻撃や移動などによって「ベンドソース」を追加し、
 * 毎フレーム経過時間を進めて不要になったものを削除する。
 *
 * 使い方:
 *   1. GrassBendManager::Initialize() で生成（草レンダラーの Init() より前に呼ぶこと）
 *   2. ModelRender に Get().GetStructuredBuffer() を SetExtraGBufferSRV で渡す
 *   3. 毎フレーム Get().Update(dt) を呼ぶ
 *   4. 攻撃時など任意のタイミングで Get().AddSource(pos, params) を呼ぶ
 *   5. シーン終了時に GrassBendManager::Finalize() を呼ぶ
 */
class GrassBendManager
{
public:
    // シェーダーの MAX_BEND_SOURCES 定義と一致させること
    static constexpr int MAX_BEND_SOURCES = 8;

    // 回転攻撃など「移動しながら連続曲げ」用の専用スロット番号
    static constexpr int SPIN_ATTACK_SLOT = MAX_BEND_SOURCES - 1;

    // 攻撃種別ごとのパラメータ (grass_attack_params.json のエントリーに対応)
    struct AttackParams
    {
        float force;         // 最大曲げ量 (ワールド単位)
        float radius;        // 影響半径 (ワールド単位)
        float duration;      // 曲げ持続時間 (秒)
        float recoverySpeed; // 将来の回復速度拡張用 (現在未使用)
    };

private:
    // GPU に送る構造体 (grass.fx の GrassBendSource と同じレイアウト)
    struct GrassBendSourceGPU
    {
        float position[3];
        float force;
        float radius;
        float elapsed;
        float duration;
        float recoverySpeed;
    };

    // CPU 側の管理用
    struct BendSource
    {
        Vector3 position      = Vector3::Zero;
        float   force         = 0.0f;
        float   radius        = 0.0f;
        float   elapsed       = 0.0f;
        float   duration      = 0.0f;
        float   recoverySpeed = 1.0f;
        bool    active        = false;
    };

    BendSource         sources_[MAX_BEND_SOURCES];
    StructuredBuffer   bendSourceSB_;
    GrassBendSourceGPU gpuData_[MAX_BEND_SOURCES] = {};

    void Init();

public:
    /** 毎フレーム呼ぶ。経過時間を進め期限切れのソースを無効化し GPU バッファを更新 */
    void Update(float dt);

    /**
     * 新しいベンドソースを追加する。
     * @param pos    攻撃・移動の中心座標 (ワールド)
     * @param params AttackParams
     */
    void AddSource(const Vector3& pos, const AttackParams& params);

    /**
     * 指定スロットを直接書き換える (elapsed をリセット)。
     * 移動しながら連続曲げを行う攻撃 (SpinAttack 等) 向け。
     * 毎フレーム呼ぶと elapsed が常に 0 にリセットされるため、
     * 呼ぶのを止めると duration 秒後に自然消滅する。
     * @param slot   SPIN_ATTACK_SLOT 等の固定スロット番号
     * @param pos    中心座標 (ワールド)
     * @param params AttackParams
     */
    void SetSource(int slot, const Vector3& pos, const AttackParams& params);

    /** GPU StructuredBuffer への参照 (ModelRender::SetExtraGBufferSRV へ渡す) */
    StructuredBuffer& GetStructuredBuffer() { return bendSourceSB_; }

    ///////////////////////////////////////////////////////////////////////////
    // シングルトン
    ///////////////////////////////////////////////////////////////////////////
private:
    static GrassBendManager* instance_;

public:
    static void Initialize()
    {
        if (instance_ == nullptr) {
            instance_ = new GrassBendManager();
            instance_->Init();
        }
    }
    static GrassBendManager& Get() { return *instance_; }
    static bool IsInitialized() { return instance_ != nullptr; }
    static void Finalize()
    {
        delete instance_;
        instance_ = nullptr;
    }
};
