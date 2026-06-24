/**
 * TitleBackground.h
 *
 * タイトル画面の3D背景・プレイヤー管理
 */

#pragma once
#include "src/Stage/StaticObject.h"
#include "src/Camera/CameraController.h"
#include "src/Memory/Array.h"
#include "src/collision/BoundingVolume.h"

// 配列サイズ（コンパイル時定数）
// スポーン済みのオブジェクトがデスポーンするまでの間に次のグループが出始めるため、
// maxConsecutive より余裕を持たせた値にする
namespace TitleBGCount {
    constexpr int TREE_COUNT  = 16;
    constexpr int TREE_COUNT2 = 16;  // 2列目の木
    constexpr int GRASS_COUNT = 14;
    constexpr int FENCE_COUNT = 14;
}


class TitleBackground : public IGameObject
{
private:
    /** JSON から読み込む全パラメーター。デフォルト値は JSON が見つからない場合のフォールバック */
    struct Param {
        float treeSpeed     = 100.0f;
        float grassSpeed    = 130.0f;
        float fenceSpeed    = 150.0f;
        float treeSpacing   = 300.0f;
        float treeZ         = -300.0f;
        float treeZ2        = -1100.0f; // 2列目の木のZ座標
        float grassSpacing  = 240.0f;
        float grassZ        = -100.0f;
        float fenceSpacing  = 250.0f;
        float fenceZ        = -180.0f;
        float groundY       = -30.0f;
        float groundScale   = 5000.0f;
        float cullingMargin  = 100.0f;
        float cullingMargin2 = 800.0f; // 奥の2列目の木専用（遠いほど広いマージンが必要）
        float spawnX        = 1000.0f;  // 右端スポーン座標（左端デスポーンは -spawnX）
        float treeSpawnX    = 0.0f;     // 木専用スポーン座標 (0 = spawnX を使用)
        float playerX       = -250.0f;
        float playerY       = 0.0f;
        float playerZ       = 0.0f;
        float playerRotYDeg = 90.0f;
        float camPosX       = 0.0f;
        float camPosY       = 150.0f;
        float camPosZ       = 800.0f;
        float camTargetX    = 0.0f;
        float camTargetY    = 0.0f;
        float camTargetZ    = 0.0f;
        float camFovDeg     = 60.0f;
        float camNear       = 1.0f;
        float camFar        = 5000.0f;
        float skyCubeScale    = 1000.0f;
        float playerRunSpeed  = 300.0f;  // 走り去り時のX移動速度

        // グループランダム配置
        // gap = baseGap * rand(0..maxGap)  ← maxGap は乗数
        int   treeMaxConsecutive  = 8;
        float treeMinGap          = 150.0f;
        float treeBaseGap         = 300.0f;
        float treeMaxGap          = 3.0f;
        int   grassMaxConsecutive = 4;
        float grassMinGap         = 50.0f;
        float grassBaseGap        = 240.0f;
        float grassMaxGap         = 2.0f;
        int   fenceMaxConsecutive = 3;
        float fenceMinGap         = 150.0f;
        float fenceBaseGap        = 250.0f;
        float fenceMaxGap         = 3.0f;
    };

    struct ScrollModel {
        ModelRender model;
        Vector3     position = Vector3::Zero;
        Quaternion  rotation = Quaternion::Identity;
        bool        active   = false;  // スポーン済みで移動中か
        bool        isDraw   = false;  // フラスタムカリング結果
    };

    /**
     * スポーナー状態機械
     * Gap  フェーズ：グループ間の間隔を待つ
     * Group フェーズ：グループ内のオブジェクトを順次スポーン
     */
    struct SpawnController {
        enum class Phase { Gap, Group };
        Phase phase     = Phase::Gap;
        int   groupLeft = 0;
        float timer     = 0.0f;  // 次のイベントまでの残り秒数
    };

private:
    Param   param_;

    StaticObject    ground_;
    ScrollModel     trees_  [TitleBGCount::TREE_COUNT];
    ScrollModel     trees2_ [TitleBGCount::TREE_COUNT2]; // 2列目
    ScrollModel     grasses_[TitleBGCount::GRASS_COUNT];
    ScrollModel     fences_ [TitleBGCount::FENCE_COUNT];

    ModelRender                   playerModel_;
    AllocatedArray<AnimationClip> playerAnims_;
    Transform                     playerTransform_;

    std::shared_ptr<GameCamera>   titleCamera_;
    SkyCube*                      skyCube_ = nullptr;


    SpawnController treeSpawn_;
    SpawnController treeSpawn2_; // 2列目
    SpawnController grassSpawn_;
    SpawnController fenceSpawn_;

    bool scrollEnabled_ = true;   // false の間はスポーン・スクロールを停止
    bool playerRunOff_  = false;  // true になるとプレイヤーが左へ走り去る
    bool playerGone_    = false;  // プレイヤーが画面外に出たら true
    bool visible_       = true;   // false の間は Render を全スキップ（Loading 中の負荷軽減）

public:
    TitleBackground();
    ~TitleBackground();

    bool Start() override;
    void Update() override;
    void Render(RenderContext& rc) override;

    /** 木・草・柵のスクロールを止める/再開する */
    void SetScrollEnabled(bool enabled) { scrollEnabled_ = enabled; }
    /** プレイヤーを左に走り去らせる */
    void StartPlayerRunOff() { playerRunOff_ = true; }
    /** プレイヤーが画面左外に出たか */
    bool IsPlayerGone() const { return playerGone_; }
    /** プレイヤー位置・スポーナーを初期状態にリセット（スクロール状態は変えない） */
    void ResetBackground();
    /** 描画をすべてスキップする（Loading 中の負荷軽減用） */
    void SetVisible(bool v) { visible_ = v; }

private:
    void LoadParam();
    void ApplyCamera();
    void ApplyPlayerTransform();
    void ResetSpawners();
    void PreFillScene();
    void UpdateSpawner(SpawnController& ctrl, ScrollModel* objects, int count,
                       float speed, float baseSpacing,
                       float minGap, float baseGap, float maxGap, int maxConsecutive,
                       float spawnX, float z, bool randomRotY = false);
    void ScrollActiveObjects(ScrollModel* objects, int count, float speed, float despawnX);
    void UpdateCulling();
};
