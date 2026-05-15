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
            obj->Init(assetPath.c_str(), data.position, data.rotation, data.scale);

            if      (data.ForwardMatchName(L"ground")) obj->SetDitherAlpha(DITHERING_ALPHA_GROUND);
            else if (data.ForwardMatchName(L"fence"))  obj->SetDitherAlpha(DITHERING_ALPHA_FENCE);
            else if (data.ForwardMatchName(L"Tree"))   obj->SetDitherAlpha(DITHERING_ALPHA_TREE);

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
