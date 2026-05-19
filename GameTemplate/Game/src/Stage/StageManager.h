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
    std::vector<StaticObject*>          staticObjectList_;
    std::vector<PhysicalBody*>          collisionList_;
    std::unique_ptr<StageCullingSystem> stageCullingSystem_;
    std::unique_ptr<GrassObject>        grassObject_;

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
    bool isDisableGlass = false;
#endif

    void StageTKLLoader(const char* path);

public:
    /** 草生成範囲の座標を取得 */
    Vector3 GetGrassAreaPos(int index) const { return grassAreaPos_[index]; }

    /** 草のLOD切り替えの有効/無効 */
    void SetGrassLodEnabled(bool v)    { if (grassObject_) grassObject_->SetLodEnabled(v);    }
    /** 草LOD遷移ディザリングの有効/無効 */
    void SetGrassDitherEnabled(bool v) { if (grassObject_) grassObject_->SetDitherEnabled(v); }

public:
    StageManager();
    ~StageManager();

    bool Start();
    void Update();
    void Render(RenderContext& rc);

#if defined(_DEBUG)
    void DisableGlass() { isDisableGlass = true; }
#endif

/**
 * シングルトン
 */
private:
    static StageManager* instance_;
    static bool          s_disableGrassLoad_;

public:
    static void SetDisableGrassLoad(bool v) { s_disableGrassLoad_ = v; }

    static void Initialize()
    {
        if (instance_ == nullptr)
            instance_ = new StageManager();
    }
    static StageManager& Get() { return *instance_; }
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
    bool isUpdate_ = true;

public:
    StageManagerObject();
    ~StageManagerObject();

    bool Start();
    void Update();
    void Render(RenderContext& rc);

    void SetUpdate(const bool flg) { isUpdate_ = flg; }
};
