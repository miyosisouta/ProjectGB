/**
 * StageManager.h
 *
 * ステージ全体の管理（静的オブジェクト・コリジョン・草）。
 * 草のLOD/ディザリング処理は GrassObject に委譲している。
 */

#pragma once
#include "src/Stage/StageBase.h"
#include "src/Stage/StaticObject.h"
#include "src/Stage/StageCullingSystem.h"
#include "src/Stage/GrassObject.h"
#include "src/collision/PhysicalBody.h"

class StageManager
{
private:
    std::vector<StaticObject*>          staticObjectList_; //!< ステージの静的オブジェクトリスト
    std::vector<PhysicalBody*>          collisionList_; //!< ステージの物理コリジョンリスト
    std::unique_ptr<StageCullingSystem> stageCullingSystem_; //!< 静的オブジェクトのカリングシステム
    std::unique_ptr<GrassObject>        grassObject_; //!< 草のLOD付きインスタンスレンダラー

    Vector3 grassAreaPos_[2]; //!< 草生成範囲の座標（外部から参照される）

    // 地面スプラットシェーダー用テクスチャ (Start() で一度だけロード)
    // TODO: 各テクスチャを PNG から DDS に変換して差し替えること
    //   kusa_ground.png   → Assets/Objects/Stage/Forest/ObjectData/kusa_ground.DDS
    //   tuthi_ground.png  → Assets/Objects/Stage/Forest/ObjectData/tuthi_ground.DDS
    //   fuyoudo_ground.png→ Assets/Objects/Stage/Forest/ObjectData/fuyoudo_ground.DDS
    //   splat_map.png     → Assets/Objects/Stage/Forest/ObjectData/splat_map.DDS
    //   (変換コマンド例: texconv <file>.png -f BC3_UNORM -o <outdir>)
    Texture splatTex_;    //!< スプラットマップ (R=草, G=岩土, B=腐葉土)
    Texture kusaTex_;     //!< 草テクスチャ
    Texture tuthiTex_;    //!< 岩土テクスチャ
    Texture fuyoudoTex_;  //!< 腐葉土テクスチャ

#if defined(_DEBUG)
    bool isDisableGlass = false; //!< デバッグ用: 草の生成を無効化するか
#endif

    /** TKLファイルからステージの静的オブジェクト・コリジョンを読み込む */
    void StageTKLLoader(const char* path);

public:
    /** 草生成範囲の座標を取得 */
    inline Vector3 GetGrassAreaPos(int index) const { return grassAreaPos_[index]; }

    /** 草のLOD切り替えの有効/無効 */
    void SetGrassLodEnabled(bool v)    { if (grassObject_) grassObject_->SetLodEnabled(v);    }
    /** 草LOD遷移ディザリングの有効/無効 */
    void SetGrassDitherEnabled(bool v) { if (grassObject_) grassObject_->SetDitherEnabled(v); }

public:
    /** コンストラクタ */
    StageManager();
    /** デストラクタ */
    ~StageManager();

    /** スタート処理 */
    bool Start();
    /** 更新処理 */
    void Update();
    /** 描画処理 */
    void Render(RenderContext& rc);

#if defined(_DEBUG)
    /** デバッグ用: 草の生成を無効化する */
    inline void DisableGlass() { isDisableGlass = true; }
#endif

/**
 * シングルトン
 */
private:
    static StageManager* instance_; //!< インスタンス
    static bool          s_disableGrassLoad_; //!< 草JSONの読み込みを無効化するか

public:
    /** 草JSONの読み込み無効化フラグを設定 */
    static inline void SetDisableGrassLoad(bool v) { s_disableGrassLoad_ = v; }

    /** インスタンスを生成 */
    static void Initialize()
    {
        if (instance_ == nullptr)
            instance_ = new StageManager();
    }
    /** インスタンスを取得 */
    static StageManager& Get() { return *instance_; }
    /** インスタンスを破棄 */
    static void Finalize()
    {
        if (instance_ != nullptr) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};


class StageManagerObject : public IGameObject
{
private:
    bool isUpdate_ = true; //!< 更新するかどうかのフラグ

public:
    /** コンストラクタ */
    StageManagerObject();
    /** デストラクタ */
    ~StageManagerObject();

    /** スタート処理 */
    bool Start();
    /** 更新処理 */
    void Update();
    /** 描画処理 */
    void Render(RenderContext& rc);

    /** 更新の可否状態を設定 */
    inline void SetUpdate(const bool flg) { isUpdate_ = flg; }
};
