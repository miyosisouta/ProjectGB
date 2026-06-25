/**
 * TitleBackground.cpp
 *
 * タイトル画面の3D背景・プレイヤー管理
 */

#include "stdafx.h"
#include "TitleBackground.h"
#include "src/Camera/CameraManager.h"
#include "src/Core/ParameterManager.h"

static constexpr const char* TITLEBG_PARAM_JSON = "Assets/Parameter/TitleBackgroundParameter.json";

namespace
{
    /* 地面 */
    constexpr float GROUND_INIT_POS = 0.0f;
    constexpr float GROUND_INIT_SCALY = 1.0f;

    /* プレイヤー */
    constexpr uint8_t PLAYER_ANIMATION_TOTAL_NUM = 2;
    constexpr uint8_t PLAYER_ANIMATION_WALK = 0;
    constexpr uint8_t PLAYER_ANIMATION_RUN = 1;
    static const Vector3 PLAYER_RUN_EFFECT_SCAL = Vector3(3.0f, 3.0f, 3.0f);
}


TitleBackground::TitleBackground() {}


TitleBackground::~TitleBackground()
{
    DeleteGO(skyCube_);
}


bool TitleBackground::Start()
{
    LoadParam();

    // 地面
    ground_.SetName("titleGround");
    ground_.Init(
        "Assets/Objects/Stage/Forest/ObjectData/titleGround.tkm",
        Vector3(GROUND_INIT_POS, param_.groundY, GROUND_INIT_POS),
        Quaternion::Identity,
        Vector3(param_.groundScale, GROUND_INIT_SCALY, param_.groundScale)
    );

    // モデル初期化（配置はスポーナーが行う）
    for (int i = 0; i < TitleBGCount::TREE_COUNT; i++)
        trees_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/tree.tkm");
    for (int i = 0; i < TitleBGCount::TREE_COUNT_SECOND; i++)
        trees2_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/tree.tkm");
    for (int i = 0; i < TitleBGCount::GRASS_COUNT; i++)
        grasses_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/grass.tkm");
    for (int i = 0; i < TitleBGCount::FENCE_COUNT; i++)
        fences_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/fence.tkm");

    ResetSpawners();
    PreFillScene();

    // プレイヤー
    {
        playerAnims_.Create(PLAYER_ANIMATION_TOTAL_NUM);
        playerAnims_[PLAYER_ANIMATION_WALK].Load("Assets/Objects/Player/Animation/Walk.tka");
        playerAnims_[PLAYER_ANIMATION_RUN].Load("Assets/Objects/Player/Animation/Run.tka");
        playerAnims_[PLAYER_ANIMATION_WALK].SetLoopFlag(true);
        playerAnims_[PLAYER_ANIMATION_RUN].SetLoopFlag(true);

        playerModel_.Init(
            "Assets/Objects/Player/Model/Model.tkm",
            playerAnims_.data(),
            static_cast<int>(playerAnims_.size()),
            enModelUpAxisZ
        );
        playerModel_.PlayAnimation(PLAYER_ANIMATION_WALK);

        ApplyPlayerTransform();
    }

    // カメラ（初回のみ SwitchCamera でアクティブ登録）
    titleCamera_ = std::make_shared<GameCamera>();
    ApplyCamera();
    CameraManager::Get().SwitchCamera(titleCamera_);

    // スカイキューブ
    skyCube_ = NewGO<SkyCube>(0, "titleSkyCube");
    skyCube_->SetType(enSkyCubeType_Day);
    skyCube_->SetScale(param_.skyCubeScale);

    return true;
}


void TitleBackground::Update()
{
#ifdef _DEBUG
    // Selectボタンで JSON をホットリロード
    if (g_pad[0]->IsTrigger(enButtonSelect)) {
        LoadParam();
        ResetSpawners();
        PreFillScene();
        ApplyPlayerTransform();
        ApplyCamera();
        skyCube_->SetScale(param_.skyCubeScale);
    }
#endif 

    // スクロール中
    if (scrollEnabled_) {
        const float treeDespawnX = -param_.tree.spawnX;
        const float despawnX     = -param_.grass.spawnX;

        UpdateSpawner(treeSpawn_,  trees_,   TitleBGCount::TREE_COUNT,  param_.tree);
        UpdateSpawner(treeSpawn2_, trees2_,  TitleBGCount::TREE_COUNT_SECOND, param_.tree2);
        UpdateSpawner(grassSpawn_, grasses_, TitleBGCount::GRASS_COUNT, param_.grass);
        UpdateSpawner(fenceSpawn_, fences_,  TitleBGCount::FENCE_COUNT, param_.fence);

        ScrollActiveObjects(trees_,   TitleBGCount::TREE_COUNT,  param_.tree.speed,  treeDespawnX);
        ScrollActiveObjects(trees2_,  TitleBGCount::TREE_COUNT_SECOND, param_.tree.speed,  treeDespawnX);
        ScrollActiveObjects(grasses_, TitleBGCount::GRASS_COUNT, param_.grass.speed, despawnX);
        ScrollActiveObjects(fences_,  TitleBGCount::FENCE_COUNT, param_.fence.speed, despawnX);
    }

    // カリング
    UpdateCulling();

    // プレイヤーが走り去り中なら右へ移動し続ける
    if (playerRunOff_) {
        const float dt = g_gameTime->GetFrameDeltaTime();
        playerModel_.PlayAnimation(PLAYER_ANIMATION_RUN);
        playerTransform_.localPosition.x += param_.playerRunSpeed * dt;
        playerTransform_.UpdateTransform();
        playerModel_.SetPosition(playerTransform_.position);
        playerModel_.SetRotation(playerTransform_.rotation);

        EffectManager::Get().PlayEffect(
            enEffectKind_Dash_Wind,
            playerTransform_.position,
            playerTransform_.rotation,
            PLAYER_RUN_EFFECT_SCAL
        );
    }

    playerModel_.Update();
}


void TitleBackground::Render(RenderContext& rc)
{
    if (!visible_) return;

    ground_.Render(rc);

    for (int i = 0; i < TitleBGCount::TREE_COUNT; i++) {
        if (trees_[i].active && trees_[i].isDraw) trees_[i].model.Draw(rc);
    }
    for (int i = 0; i < TitleBGCount::TREE_COUNT_SECOND; i++) {
        if (trees2_[i].active && trees2_[i].isDraw) trees2_[i].model.Draw(rc);
    }
    for (int i = 0; i < TitleBGCount::GRASS_COUNT; i++) {
        if (grasses_[i].active && grasses_[i].isDraw) grasses_[i].model.Draw(rc);
    }
    for (int i = 0; i < TitleBGCount::FENCE_COUNT; i++) {
        if (fences_[i].active && fences_[i].isDraw) fences_[i].model.Draw(rc);
    }

    playerModel_.Draw(rc);
}


void TitleBackground::ResetBackground()
{
    playerRunOff_ = false; // スクロール停止
    playerGone_   = false; // 走り始める
    playerModel_.PlayAnimation(PLAYER_ANIMATION_WALK);
    ResetSpawners();
    PreFillScene();
    ApplyPlayerTransform();
}


void TitleBackground::LoadParam()
{
    // ホットリロード対応：既存データを破棄してから再読み込み
    ParameterManager::Get().UnloadParameter<MasterTitleBGParameter>();
    ParameterManager::Get().LoadTitleBGParamData(TITLEBG_PARAM_JSON);

    const auto* p = ParameterManager::Get().GetTitleBGParam();
    if (!p) return;

    const float treeSpawnX = p->treeSpawnX > 0.0f ? p->treeSpawnX : p->spawnX;

    param_.tree = {
        p->treeSpeed,  p->treeSpacing,
        p->treeMinGap, p->treeBaseGap, p->treeMaxGap, p->treeMaxConsecutive,
        treeSpawnX,    p->treeZ,       true
    };
    param_.tree2       = param_.tree;
    param_.tree2.z     = p->treeZ2;

    param_.grass = {
        p->grassSpeed,  p->grassSpacing,
        p->grassMinGap, p->grassBaseGap, p->grassMaxGap, p->grassMaxConsecutive,
        p->spawnX,      p->grassZ,       true
    };

    param_.fence = {
        p->fenceSpeed,  p->fenceSpacing,
        p->fenceMinGap, p->fenceBaseGap, p->fenceMaxGap, p->fenceMaxConsecutive,
        p->spawnX,      p->fenceZ,       false
    };

    param_.groundY        = p->groundY;
    param_.groundScale    = p->groundScale;
    param_.cullingMargin  = p->cullingMargin;
    param_.playerX        = p->playerX;
    param_.playerY        = p->playerY;
    param_.playerZ        = p->playerZ;
    param_.playerRotYDeg  = p->playerRotYDeg;
    param_.camPosX        = p->camPosX;
    param_.camPosY        = p->camPosY;
    param_.camPosZ        = p->camPosZ;
    param_.camTargetX     = p->camTargetX;
    param_.camTargetY     = p->camTargetY;
    param_.camTargetZ     = p->camTargetZ;
    param_.camFovDeg      = p->camFovDeg;
    param_.camNear        = p->camNear;
    param_.camFar         = p->camFar;
    param_.skyCubeScale   = p->skyCubeScale;
    param_.playerRunSpeed = p->playerRunSpeed;
}


void TitleBackground::ApplyCamera()
{
    CameraData camData;
    camData.position = Vector3(param_.camPosX, param_.camPosY, param_.camPosZ);
    camData.target   = Vector3(param_.camTargetX, param_.camTargetY, param_.camTargetZ);
    camData.fov      = Math::DegToRad(param_.camFovDeg);
    camData.nearClip = param_.camNear;
    camData.farClip  = param_.camFar;

    // SetState だけで十分（CameraManager::Update が毎フレーム GetCameraData() を読む）
    // SwitchCamera は Start() 時の初回のみ呼ぶ
    titleCamera_->SetState(camData);
}


void TitleBackground::ApplyPlayerTransform()
{
    Quaternion rot;
    rot.SetRotationY(Math::DegToRad(param_.playerRotYDeg));

    playerTransform_.localPosition = Vector3(param_.playerX, param_.playerY, param_.playerZ);
    playerTransform_.localRotation = rot;
    playerTransform_.localScale    = Vector3::One;
    playerTransform_.UpdateTransform();

    playerModel_.SetPosition(playerTransform_.position);
    playerModel_.SetRotation(playerTransform_.rotation);
    playerModel_.Update();
}


void TitleBackground::PreFillScene()
{
    // UpdateSpawner と同じグループ/ギャップのランダムロジックで
    // 右端 → 左端へ向かってオブジェクトを配置する
    auto fill = [](
        ScrollModel* objects, int count, const SpawnConfig& cfg,
        SpawnController& ctrl, float despawnX)
    {
        float x   = cfg.spawnX;
        int   idx = 0;

        while (x > despawnX && idx < count) {
            const int groupSize = 1 + rand() % cfg.maxConsecutive;

            for (int g = 0; g < groupSize && idx < count && x > despawnX; g++, idx++) {
                objects[idx].position = Vector3(x, 0.0f, cfg.z);
                if (cfg.randomRotY) {
                    const float rotDeg = static_cast<float>(1 + rand() % 10) * 18.0f;
                    objects[idx].rotation.SetRotationY(Math::DegToRad(rotDeg));
                }
                objects[idx].model.SetPosition(objects[idx].position);
                objects[idx].model.SetRotation(objects[idx].rotation);
                objects[idx].model.Update();
                objects[idx].active = true;
                objects[idx].isDraw = true;
                x -= cfg.spacing;
            }

            const float effectiveMinGap = (std::max)(cfg.minGap, cfg.spacing);
            const float gap = effectiveMinGap + cfg.baseGap * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * cfg.maxGap;
            x -= gap;
        }

        // スポーナーを Gap フェーズで再開し、次のグループまで待機
        const float effectiveMinGap = (std::max)(cfg.minGap, cfg.spacing);
        const float initGap = effectiveMinGap + cfg.baseGap * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * cfg.maxGap;
        ctrl.phase     = SpawnController::Phase::Gap;
        ctrl.groupLeft = 0;
        ctrl.timer     = initGap / cfg.speed;
    };

    const float treeDespawnX = -param_.tree.spawnX;
    const float despawnX     = -param_.grass.spawnX;

    fill(trees_,   TitleBGCount::TREE_COUNT,  param_.tree,  treeSpawn_,  treeDespawnX);
    fill(trees2_,  TitleBGCount::TREE_COUNT_SECOND, param_.tree2, treeSpawn2_, treeDespawnX);
    fill(grasses_, TitleBGCount::GRASS_COUNT, param_.grass, grassSpawn_, despawnX);
    fill(fences_,  TitleBGCount::FENCE_COUNT, param_.fence, fenceSpawn_, despawnX);
}


void TitleBackground::ResetSpawners()
{
    // 全オブジェクトを非アクティブにしてスポーナーをリセット
    for (auto& t : trees_)   { t.active = false; t.isDraw = false; }
    for (auto& t : trees2_)  { t.active = false; t.isDraw = false; }
    for (auto& g : grasses_) { g.active = false; g.isDraw = false; }
    for (auto& f : fences_)  { f.active = false; f.isDraw = false; }

    treeSpawn_  = SpawnController{};
    treeSpawn2_ = SpawnController{};
    grassSpawn_ = SpawnController{};
    fenceSpawn_ = SpawnController{};
}


void TitleBackground::UpdateSpawner(
    SpawnController& ctrl, ScrollModel* objects, int count, const SpawnConfig& cfg)
{
    const float dt = g_gameTime->GetFrameDeltaTime();
    ctrl.timer -= dt;
    if (ctrl.timer > 0.0f) return;

    // Gap フェーズ終了 → 新グループ開始
    if (ctrl.phase == SpawnController::Phase::Gap) {
        ctrl.phase     = SpawnController::Phase::Group;
        ctrl.groupLeft = 1 + rand() % cfg.maxConsecutive;
    }

    // 非アクティブなスロットを探してスポーン
    for (int i = 0; i < count; i++) {
        if (!objects[i].active) {
            objects[i].position = Vector3(cfg.spawnX, 0.0f, cfg.z);
            if (cfg.randomRotY) {
                const float rotDeg = static_cast<float>(1 + rand() % 10) * 18.0f;
                objects[i].rotation.SetRotationY(Math::DegToRad(rotDeg));
            }
            objects[i].model.SetPosition(objects[i].position);
            objects[i].model.SetRotation(objects[i].rotation);
            objects[i].model.Update();
            objects[i].active = true;
            objects[i].isDraw = true;
            break;
        }
    }

    ctrl.groupLeft--;

    if (ctrl.groupLeft > 0) {
        // グループ継続：spacing 分進む時間後に次をスポーン
        ctrl.timer = cfg.spacing / cfg.speed;
    } else {
        // グループ終了 → Gap フェーズへ
        ctrl.phase = SpawnController::Phase::Gap;
        const float effectiveMinGap = (std::max)(cfg.minGap, cfg.spacing);
        const float gap = effectiveMinGap + cfg.baseGap * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * cfg.maxGap;
        ctrl.timer = gap / cfg.speed;
    }
}


void TitleBackground::ScrollActiveObjects(
    ScrollModel* objects, int count, float speed, float despawnX)
{
    const float dt = g_gameTime->GetFrameDeltaTime();
    for (int i = 0; i < count; i++) {
        if (!objects[i].active) continue;
        objects[i].position.x -= speed * dt;
        objects[i].model.SetPosition(objects[i].position);
        objects[i].model.SetRotation(objects[i].rotation);
        objects[i].model.Update();
        // 左端を超えたら非アクティブ（スポーナーが次のタイミングで再利用）
        if (objects[i].position.x < despawnX) {
            objects[i].active = false;
            objects[i].isDraw = false;
        }
    }
}


void TitleBackground::UpdateCulling()
{
    Frustum frustum;
    frustum.BuildFromViewProjectionMatrix(g_camera3D->GetViewProjectionMatrix());

    const Vector3 margin(param_.cullingMargin, param_.cullingMargin, param_.cullingMargin);

    for (int i = 0; i < TitleBGCount::TREE_COUNT; i++) {
        if (!trees_[i].active) continue;
        Bounds bounds;
        bounds.Compute(trees_[i].model.GetModel());
        bounds.maxPoint += trees_[i].position + margin;
        bounds.minPoint += trees_[i].position - margin;
        trees_[i].isDraw = frustum.IsVisible(bounds);
    }

    for (int i = 0; i < TitleBGCount::TREE_COUNT_SECOND; i++) {
        if (!trees2_[i].active) continue;
        Bounds bounds;
        bounds.Compute(trees2_[i].model.GetModel());
        bounds.maxPoint += trees2_[i].position + margin;
        bounds.minPoint += trees2_[i].position - margin;
        trees2_[i].isDraw = frustum.IsVisible(bounds);
    }

    for (int i = 0; i < TitleBGCount::GRASS_COUNT; i++) {
        if (!grasses_[i].active) continue;
        Bounds bounds;
        bounds.Compute(grasses_[i].model.GetModel());
        bounds.maxPoint += grasses_[i].position + margin;
        bounds.minPoint += grasses_[i].position - margin;
        grasses_[i].isDraw = frustum.IsVisible(bounds);
    }

    for (int i = 0; i < TitleBGCount::FENCE_COUNT; i++) {
        if (!fences_[i].active) continue;
        Bounds bounds;
        bounds.Compute(fences_[i].model.GetModel());
        bounds.maxPoint += fences_[i].position + margin;
        bounds.minPoint += fences_[i].position - margin;
        fences_[i].isDraw = frustum.IsVisible(bounds);
    }
}
