#pragma once
/**
 * GrassObject.h
 * 草のLOD付きインスタンスレンダラー
 *
 * フラスタムカリング・LOD選択・LOD遷移ディザリングを一括管理する。
 * StageManagerは Init / Update / Draw を呼ぶだけでよい。
 */
#include "src/collision/BoundingVolume.h"

class GrassBendManager;

class GrassObject
{
public:
    static constexpr int LOD_COUNT = 3; //!< LODの段階数

private:
    /** 草1本分のトランスフォーム */
    struct GrassTransform
    {
        Vector3    position = Vector3::Zero; //!< 座標
        Quaternion rotation = Quaternion::Identity; //!< 回転
        Vector3    scale    = Vector3::One; //!< 拡縮
    };

    // LOD切り替え距離
    static constexpr float LOD0_MAX_DIST     = 1900.0f; //!< LOD0からLOD1へ切り替わる距離
    static constexpr float LOD1_MAX_DIST     = 2400.0f; //!< LOD1からLOD2へ切り替わる距離
    // LOD遷移ディザリング距離 (デバッグ用: 動作確認後に元の値へ戻すこと)
    static constexpr float DITHER_START_DIST = LOD0_MAX_DIST - 100.0f;  //!< ディザ遷移を開始する距離。TODO: 本来は LOD0_MAX_DIST - 100.0f
    static constexpr float DITHER_ALPHA_MIN  = 0.4f; //!< ディザ遷移中の最小アルファ値

    static const char* const LOD_MODEL_PATHS[LOD_COUNT]; //!< LODごとのモデルパス

    ModelRender                 renderer_[LOD_COUNT]; //!< LODごとのインスタンシングレンダラー
    std::vector<GrassTransform> transforms_; //!< 配置した草のトランスフォームリスト
    Bounds                      templateBounds_; //!< LOD0モデルのローカルAABB(カリング用テンプレート)

    bool isLodEnabled_    = true; //!< LOD切り替えを行うか
    bool isDitherEnabled_ = true; //!< LOD遷移ディザリングを行うか

public:
    /** コンストラクタ */
    GrassObject()  = default;
    /** デストラクタ */
    ~GrassObject() = default;

    /**
     * JSONから草を読み込んでインスタンシングデータを構築する。
     * @param jsonPath  草配置JSONのパス
     * @param bendMgr   nullptr可。LOD0/1の草曲げシェーダーを有効化したいときに渡す
     */
    void Init(const char* jsonPath, GrassBendManager* bendMgr = nullptr);

    /** フラスタムカリング・LOD選択・インスタンスデータ更新 */
    void Update();

    /** LOD別に描画 (LOD0/1は草LODディザCBを設定してから描画) */
    void Draw(RenderContext& rc);

    /** 草が1本も配置されていないか */
    inline bool IsEmpty() const { return transforms_.empty(); }

    /** LOD切り替えの有効/無効 (falseのとき常にLOD0で描画) */
    inline void SetLodEnabled(bool v)    { isLodEnabled_    = v; }
    /** LOD遷移ディザリングの有効/無効 */
    inline void SetDitherEnabled(bool v) { isDitherEnabled_ = v; }
};
