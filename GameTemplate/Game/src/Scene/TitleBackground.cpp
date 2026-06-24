/**
 * TitleBackground.cpp
 *
 * タイトル画面の3D背景・プレイヤー管理
 */

#include "stdafx.h"
#include "TitleBackground.h"
#include "src/Camera/CameraManager.h"

static constexpr const char* TITLEBG_PARAM_JSON = "Assets/Parameter/TitleBackgroundParameter.json";


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
        Vector3(0.0f, param_.groundY, 0.0f),
        Quaternion::Identity,
        Vector3(param_.groundScale, 1.0f, param_.groundScale)
    );

    // モデル初期化（配置はスポーナーが行う）
    for (int i = 0; i < TitleBGCount::TREE_COUNT; i++)
        trees_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/tree.tkm");
    for (int i = 0; i < TitleBGCount::TREE_COUNT2; i++)
        trees2_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/tree.tkm");
    for (int i = 0; i < TitleBGCount::GRASS_COUNT; i++)
        grasses_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/grass.tkm");
    for (int i = 0; i < TitleBGCount::FENCE_COUNT; i++)
        fences_[i].model.Init("Assets/Objects/Stage/Forest/ObjectData/fence.tkm");

    ResetSpawners();
    PreFillScene();

    // プレイヤー
    {
        playerAnims_.Create(1);
        playerAnims_[0].Load("Assets/Objects/Player/Animation/Walk.tka");
        playerAnims_[0].SetLoopFlag(true);

        playerModel_.Init(
            "Assets/Objects/Player/Model/Model.tkm",
            playerAnims_.data(),
            static_cast<int>(playerAnims_.size()),
            enModelUpAxisZ
        );
        playerModel_.PlayAnimation(0);

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
    // Selectボタンで JSON をホットリロード
    if (g_pad[0]->IsTrigger(enButtonSelect)) {
        LoadParam();
        ResetSpawners();
        PreFillScene();
        ApplyPlayerTransform();
        ApplyCamera();
        skyCube_->SetScale(param_.skyCubeScale);
    }

    if (scrollEnabled_) {
        const float despawnX   = -param_.spawnX;
        const float treeSpawnX = param_.treeSpawnX > 0.0f ? param_.treeSpawnX : param_.spawnX;
        const float treeDespawnX = -treeSpawnX;

        UpdateSpawner(treeSpawn_,  trees_,   TitleBGCount::TREE_COUNT,
                      param_.treeSpeed,  param_.treeSpacing,
                      param_.treeMinGap,  param_.treeBaseGap,  param_.treeMaxGap,  param_.treeMaxConsecutive,
                      treeSpawnX, param_.treeZ, true);

        UpdateSpawner(treeSpawn2_, trees2_,  TitleBGCount::TREE_COUNT2,
                      param_.treeSpeed,  param_.treeSpacing,
                      param_.treeMinGap,  param_.treeBaseGap,  param_.treeMaxGap,  param_.treeMaxConsecutive,
                      treeSpawnX, param_.treeZ2, true);

        UpdateSpawner(grassSpawn_, grasses_, TitleBGCount::GRASS_COUNT,
                      param_.grassSpeed, param_.grassSpacing,
                      param_.grassMinGap, param_.grassBaseGap, param_.grassMaxGap, param_.grassMaxConsecutive,
                      param_.spawnX, param_.grassZ, true);

        UpdateSpawner(fenceSpawn_, fences_,  TitleBGCount::FENCE_COUNT,
                      param_.fenceSpeed, param_.fenceSpacing,
                      param_.fenceMinGap, param_.fenceBaseGap, param_.fenceMaxGap, param_.fenceMaxConsecutive,
                      param_.spawnX, param_.fenceZ);

        ScrollActiveObjects(trees_,   TitleBGCount::TREE_COUNT,  param_.treeSpeed,  treeDespawnX);
        ScrollActiveObjects(trees2_,  TitleBGCount::TREE_COUNT2, param_.treeSpeed,  treeDespawnX);
        ScrollActiveObjects(grasses_, TitleBGCount::GRASS_COUNT, param_.grassSpeed, despawnX);
        ScrollActiveObjects(fences_,  TitleBGCount::FENCE_COUNT, param_.fenceSpeed, despawnX);
    }

    UpdateCulling();

    // プレイヤーが走り去り中なら左へ移動
    if (playerRunOff_ && !playerGone_) {
        const float dt = g_gameTime->GetFrameDeltaTime();
        playerTransform_.localPosition.x -= param_.playerRunSpeed * dt;
        playerTransform_.UpdateTransform();
        playerModel_.SetPosition(playerTransform_.position);
        playerModel_.SetRotation(playerTransform_.rotation);
        if (playerTransform_.localPosition.x < -param_.spawnX) {
            playerGone_ = true;
        }
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
    for (int i = 0; i < TitleBGCount::TREE_COUNT2; i++) {
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
    playerRunOff_ = false;
    playerGone_   = false;
    ResetSpawners();
    PreFillScene();
    ApplyPlayerTransform();
}


void TitleBackground::LoadParam()
{
    std::ifstream file(TITLEBG_PARAM_JSON);
    if (!file.is_open()) return;

    nlohmann::json j;
    file >> j;

    if (!j.contains("TitleBackground") || !j["TitleBackground"].is_array() || j["TitleBackground"].empty())
        return;

    const auto& p = j["TitleBackground"][0];
    param_.treeSpeed     = p.value("treeSpeed",     param_.treeSpeed);
    param_.grassSpeed    = p.value("grassSpeed",    param_.grassSpeed);
    param_.fenceSpeed    = p.value("fenceSpeed",    param_.fenceSpeed);
    param_.treeSpacing   = p.value("treeSpacing",   param_.treeSpacing);
    param_.treeZ         = p.value("treeZ",         param_.treeZ);
    param_.treeZ2        = p.value("treeZ2",        param_.treeZ2);
    param_.grassSpacing  = p.value("grassSpacing",  param_.grassSpacing);
    param_.grassZ        = p.value("grassZ",        param_.grassZ);
    param_.fenceSpacing  = p.value("fenceSpacing",  param_.fenceSpacing);
    param_.fenceZ        = p.value("fenceZ",        param_.fenceZ);
    param_.groundY       = p.value("groundY",       param_.groundY);
    param_.groundScale   = p.value("groundScale",   param_.groundScale);
    param_.cullingMargin  = p.value("cullingMargin",  param_.cullingMargin);
    param_.cullingMargin2 = p.value("cullingMargin2", param_.cullingMargin2);
    param_.spawnX        = p.value("spawnX",        param_.spawnX);
    param_.treeSpawnX    = p.value("treeSpawnX",    param_.treeSpawnX);
    param_.playerX       = p.value("playerX",       param_.playerX);
    param_.playerY       = p.value("playerY",       param_.playerY);
    param_.playerZ       = p.value("playerZ",       param_.playerZ);
    param_.playerRotYDeg = p.value("playerRotYDeg", param_.playerRotYDeg);
    param_.camPosX       = p.value("camPosX",       param_.camPosX);
    param_.camPosY       = p.value("camPosY",       param_.camPosY);
    param_.camPosZ       = p.value("camPosZ",       param_.camPosZ);
    param_.camTargetX    = p.value("camTargetX",    param_.camTargetX);
    param_.camTargetY    = p.value("camTargetY",    param_.camTargetY);
    param_.camTargetZ    = p.value("camTargetZ",    param_.camTargetZ);
    param_.camFovDeg     = p.value("camFovDeg",     param_.camFovDeg);
    param_.camNear       = p.value("camNear",       param_.camNear);
    param_.camFar        = p.value("camFar",        param_.camFar);
    param_.skyCubeScale  = p.value("skyCubeScale",  param_.skyCubeScale);

    param_.treeMaxConsecutive  = p.value("treeMaxConsecutive",  param_.treeMaxConsecutive);
    param_.treeMinGap          = p.value("treeMinGap",          param_.treeMinGap);
    param_.treeBaseGap         = p.value("treeBaseGap",         param_.treeBaseGap);
    param_.treeMaxGap          = p.value("treeMaxGap",          param_.treeMaxGap);
    param_.grassMaxConsecutive = p.value("grassMaxConsecutive", param_.grassMaxConsecutive);
    param_.grassMinGap         = p.value("grassMinGap",         param_.grassMinGap);
    param_.grassBaseGap        = p.value("grassBaseGap",        param_.grassBaseGap);
    param_.grassMaxGap         = p.value("grassMaxGap",         param_.grassMaxGap);
    param_.fenceMaxConsecutive = p.value("fenceMaxConsecutive", param_.fenceMaxConsecutive);
    param_.fenceMinGap         = p.value("fenceMinGap",         param_.fenceMinGap);
    param_.fenceBaseGap        = p.value("fenceBaseGap",        param_.fenceBaseGap);
    param_.fenceMaxGap         = p.value("fenceMaxGap",         param_.fenceMaxGap);
    param_.playerRunSpeed      = p.value("playerRunSpeed",      param_.playerRunSpeed);
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
    const float despawnX   = -param_.spawnX;
    const float treeSpawnX = param_.treeSpawnX > 0.0f ? param_.treeSpawnX : param_.spawnX;

    // UpdateSpawner と同じグループ/ギャップのランダムロジックで
    // 右端 → 左端へ向かってオブジェクトを配置する
    auto fill = [despawnX](
        ScrollModel* objects, int count,
        float rightEdge, float spacing,
        float minGap, float baseGap, float maxGap, int maxConsecutive,
        float z, SpawnController& ctrl, float speed, bool randomRotY)
    {
        float x   = rightEdge;
        int   idx = 0;

        while (x > despawnX && idx < count) {
            // グループサイズをランダム決定
            const int groupSize = 1 + rand() % maxConsecutive;

            for (int g = 0; g < groupSize && idx < count && x > despawnX; g++, idx++) {
                objects[idx].position = Vector3(x, 0.0f, z);
                if (randomRotY) {
                    const float rotDeg = static_cast<float>(1 + rand() % 10) * 18.0f;
                    objects[idx].rotation.SetRotationY(Math::DegToRad(rotDeg));
                }
                objects[idx].model.SetPosition(objects[idx].position);
                objects[idx].model.SetRotation(objects[idx].rotation);
                objects[idx].model.Update();
                objects[idx].active = true;
                objects[idx].isDraw = true;
                x -= spacing;
            }

            // グループ間ギャップ（UpdateSpawner と同じ計算式）
            const float effectiveMinGap = (std::max)(minGap, spacing);
            const float gap = effectiveMinGap + baseGap * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * maxGap;
            x -= gap;
        }

        // スポーナーを Gap フェーズで再開し、次のグループまで待機
        const float effectiveMinGap = (std::max)(minGap, spacing);
        const float initGap = effectiveMinGap + baseGap * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * maxGap;
        ctrl.phase     = SpawnController::Phase::Gap;
        ctrl.groupLeft = 0;
        ctrl.timer     = initGap / speed;
    };

    fill(trees_,   TitleBGCount::TREE_COUNT,  treeSpawnX,   param_.treeSpacing,
         param_.treeMinGap,  param_.treeBaseGap,  param_.treeMaxGap,  param_.treeMaxConsecutive,
         param_.treeZ,  treeSpawn_,  param_.treeSpeed, true);

    fill(trees2_,  TitleBGCount::TREE_COUNT2, treeSpawnX,   param_.treeSpacing,
         param_.treeMinGap,  param_.treeBaseGap,  param_.treeMaxGap,  param_.treeMaxConsecutive,
         param_.treeZ2, treeSpawn2_, param_.treeSpeed, true);

    fill(grasses_, TitleBGCount::GRASS_COUNT, param_.spawnX, param_.grassSpacing,
         param_.grassMinGap, param_.grassBaseGap, param_.grassMaxGap, param_.grassMaxConsecutive,
         param_.grassZ, grassSpawn_, param_.grassSpeed, true);

    fill(fences_,  TitleBGCount::FENCE_COUNT, param_.spawnX, param_.fenceSpacing,
         param_.fenceMinGap, param_.fenceBaseGap, param_.fenceMaxGap, param_.fenceMaxConsecutive,
         param_.fenceZ, fenceSpawn_, param_.fenceSpeed, false);
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
    SpawnController& ctrl, ScrollModel* objects, int count,
    float speed, float baseSpacing,
    float minGap, float baseGap, float maxGap, int maxConsecutive,
    float spawnX, float z, bool randomRotY)
{
    const float dt = g_gameTime->GetFrameDeltaTime();
    ctrl.timer -= dt;
    if (ctrl.timer > 0.0f) return;

    // Gap フェーズ終了 → 新グループ開始
    if (ctrl.phase == SpawnController::Phase::Gap) {
        ctrl.phase     = SpawnController::Phase::Group;
        ctrl.groupLeft = 1 + rand() % maxConsecutive;
    }

    // 非アクティブなスロットを探してスポーン
    for (int i = 0; i < count; i++) {
        if (!objects[i].active) {
            objects[i].position = Vector3(spawnX, 0.0f, z);
            if (randomRotY) {
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
        // グループ継続：baseSpacing 分進む時間後に次をスポーン
        ctrl.timer = baseSpacing / speed;
    } else {
        // グループ終了 → Gap フェーズへ
        // グループ間の最小距離はグループ内間隔 (baseSpacing) 以上を保証し、重なりを防ぐ
        ctrl.phase = SpawnController::Phase::Gap;
        const float effectiveMinGap = (std::max)(minGap, baseSpacing);
        const float gap = effectiveMinGap + baseGap * (static_cast<float>(rand()) / static_cast<float>(RAND_MAX)) * maxGap;
        ctrl.timer = gap / speed;
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

    const Vector3 margin (param_.cullingMargin,  param_.cullingMargin,  param_.cullingMargin);
    const Vector3 margin2(param_.cullingMargin2, param_.cullingMargin2, param_.cullingMargin2);

    for (int i = 0; i < TitleBGCount::TREE_COUNT; i++) {
        if (!trees_[i].active) continue;
        Bounds bounds;
        bounds.Compute(trees_[i].model.GetModel());
        bounds.maxPoint += trees_[i].position + margin;
        bounds.minPoint += trees_[i].position - margin;
        trees_[i].isDraw = frustum.IsVisible(bounds);
    }

    // 2列目は奥にある（Z=-1100）分だけ視錐台端でのX幅が広い。
    // cullingMargin2 を大きくして画面端付近での消えを防ぐ。
    for (int i = 0; i < TitleBGCount::TREE_COUNT2; i++) {
        if (!trees2_[i].active) continue;
        Bounds bounds;
        bounds.Compute(trees2_[i].model.GetModel());
        bounds.maxPoint += trees2_[i].position + margin2;
        bounds.minPoint += trees2_[i].position - margin2;
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
