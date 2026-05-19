/**
 * StageManager.cpp
 * ステージ全体の管理。草の詳細処理は GrassObject に委譲している。
 */

#include "stdafx.h"
#include "StageManager.h"
#include "GrassBendManager.h"
#include "src/CharacterDataBase.h"
#include "src/collision/PhysicalBody.h"

StageManager* StageManager::instance_        = nullptr;
bool          StageManager::s_disableGrassLoad_ = false;


namespace
{
    /* ディザリング時の透明度 */
    constexpr float DITHERING_ALPHA_GROUND = 1.0f; // 地面
    constexpr float DITHERING_ALPHA_FENCE  = 0.5f; // 柵
    constexpr float DITHERING_ALPHA_TREE   = 0.2f; // 木

    /* スプラットマップ */
    constexpr float SPLAT_MAP_GRASS_LUMINANCE = 1.0f;   // 草の明るさ
    constexpr float SPLAT_MAP_CRAY_LUMINANCE = 1.0f;    // 土の明るさ
    constexpr float SPLAT_MAP_HUMUS_LUMINANCE = 1.0f;   // 腐葉土の明るさ
    constexpr float SPLAT_MAP_WHOLE_LUMINANCE = 3.0f;   // 全体の明るさ
    constexpr float SPLAT_MAP_SATURATION = 2.5f; // 彩度

    /* その他 */
    constexpr float COLLISION_UP        = 600.0f; // コリジョンの高さ
    constexpr float METER_TO_CENTIMETER = 100.0f; // メートル

    /* 草の配置JSONパス */
    const char* GRASS_JSON_PATH = "Assets/Objects/Stage/Forest/ObjectData/grass_placement.json";
}


void StageManager::StageTKLLoader(const char* path)
{
    LevelRender stage;
    stage.Init(path, [&](LevelObjectData& data)
    {
        std::string assetPath;

        if (data.ForwardMatchName(L"ground")) {
            assetPath = "Assets/Objects/Stage/Forest/ObjectData/ground.tkm";
        }
        else if (data.ForwardMatchName(L"fence")) {
            assetPath = "Assets/Objects/Stage/Forest/ObjectData/fence.tkm";
        }
        else if (data.ForwardMatchName(L"Tree")) {
            assetPath = "Assets/Objects/Stage/Forest/ObjectData/tree.tkm";
        }
        else if (data.ForwardMatchName(L"collisionBox")) {
            auto collision = new PhysicalBody();
            Vector3 pos = Vector3(data.position.x, data.position.y + COLLISION_UP, data.position.z);
            collision->CreateBox(data.scale * METER_TO_CENTIMETER, pos, enCollisionAttr_Ground);
            collisionList_.push_back(collision);
            return true;
        }
        else if (data.ForwardMatchName(L"GrassArea")) {
            if      (data.EqualObjectName(L"GrassArea01")) grassAreaPos_[0] = data.position;
            else if (data.EqualObjectName(L"GrassArea02")) grassAreaPos_[1] = data.position;
        }

        if (!assetPath.empty()) {
            auto* obj = new StaticObject();

            // スプラットシェーダーは Init() より前にセットアップする必要がある
            if (data.ForwardMatchName(L"ground")) {
                obj->SetupSplatShader(&splatTex_, &kusaTex_, &tuthiTex_, &fuyoudoTex_);
            }

            obj->Init(assetPath.c_str(), data.position, data.rotation, data.scale);

            // 固有値の設定
            if (data.ForwardMatchName(L"ground")) {
                obj->SetDitherAlpha(DITHERING_ALPHA_GROUND); // ディザのアルファ値
                obj->SetName("ground"); // 名前
                // スプラットカラー調整 (p0=草, p1=岩土, p2=腐葉土の明るさ倍率, p3=全体HSV明るさ)
                obj->SetSplatColorParams(SPLAT_MAP_GRASS_LUMINANCE, SPLAT_MAP_CRAY_LUMINANCE, SPLAT_MAP_HUMUS_LUMINANCE, SPLAT_MAP_WHOLE_LUMINANCE);
                // スプラット彩度調整 (1.0=変化なし, 1.5=鮮やか)
                obj->SetSplatSaturation(SPLAT_MAP_SATURATION);
            }
            else if (data.ForwardMatchName(L"fence")) {
                obj->SetDitherAlpha(DITHERING_ALPHA_FENCE); // ディザのアルファ値
                obj->SetName("fence"); // 名前
            }
            else if (data.ForwardMatchName(L"Tree")) {
                obj->SetDitherAlpha(DITHERING_ALPHA_TREE); // ディザのアルファ値
                obj->SetName("Tree"); // 名前
            }

            staticObjectList_.push_back(obj);
        }
    });
}


StageManager::StageManager() {}

StageManager::~StageManager()
{
    for (auto* obj : staticObjectList_) delete obj;
    staticObjectList_.clear();
    for (auto* obj : collisionList_) delete obj;
    collisionList_.clear();
}


bool StageManager::Start()
{
    // 地面スプラットシェーダー用テクスチャをロード (StageTKLLoader より前に行う)
    // splat_map の各チャンネル: R=草, G=岩土, B=腐葉土
    // miniRed.DDS (R=1,G=0,B=0) をプレースホルダーとして使用 → 草テクスチャのみ表示
    splatTex_.InitFromDDSFile(L"Assets/Objects/Stage/Forest/ObjectData/SplatMap.DDS");
    kusaTex_.InitFromDDSFile(L"Assets/Objects/Stage/Forest/ObjectData/kusaGround.DDS");
    tuthiTex_.InitFromDDSFile(L"Assets/Objects/Stage/Forest/ObjectData/tuthi_ground.DDS");
    fuyoudoTex_.InitFromDDSFile(L"Assets/Objects/Stage/Forest/ObjectData/fuyoudo_ground.DDS");
    // 選択されたボスを取得
    //BossType stageKind = CharacterDataBase::Get().GetGameParam().stageType_;
    BossType stageKind = BossType::enGorilla;

    switch (stageKind)
    {
    case BossType::enGorilla:
        StageTKLLoader("Assets/Objects/Stage/Forest/tkl/Stage_Gollira.tkl");
        break;
    case BossType::enTurtle:
        StageTKLLoader("Assets/Objects/Stage/Forest/tkl/Stage_Turtle.tkl");
        break;
    default:
        K2_ASSERT(true, "ボスタイプが設定されていません");
        break;
    }

    stageCullingSystem_ = std::make_unique<StageCullingSystem>();

    if (!s_disableGrassLoad_)
    {
#if defined(_DEBUG)
        if (!isDisableGlass)
#endif
        {
            // グラスオブジェクトを作成
            grassObject_ = std::make_unique<GrassObject>();
            grassObject_->Init(
                GRASS_JSON_PATH,
                GrassBendManager::IsInitialized() ? &GrassBendManager::Get() : nullptr
            );
        }
    }

    return true;
}


void StageManager::Update()
{
    stageCullingSystem_->Update(staticObjectList_);
    if (grassObject_) grassObject_->Update();
}


void StageManager::Render(RenderContext& rc)
{
    for (auto* obj : staticObjectList_) obj->Render(rc);
    if (grassObject_) grassObject_->Draw(rc);
}


/******************* StageManagerObject ********************************/

StageManagerObject::StageManagerObject() {}

StageManagerObject::~StageManagerObject()
{
    StageManager::Get().Finalize();
}

bool StageManagerObject::Start()
{
    StageManager::Initialize();
    StageManager::Get().Start();

    // TODO : 当たり判定の可視化
#ifdef K2_DEBUG
    //PhysicsWorld::Get().EnableDrawDebugWireFrame();
#endif

    return true;
}

void StageManagerObject::Update()
{
    if (!isUpdate_) return;
    StageManager::Get().Update();
}

void StageManagerObject::Render(RenderContext& rc)
{
    StageManager::Get().Render(rc);
}
