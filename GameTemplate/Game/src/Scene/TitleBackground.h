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
    constexpr int TREE_COUNT  = 16;         //!< 1列目の木
    constexpr int TREE_COUNT_SECOND = 16;   //!< 2列目の木
    constexpr int GRASS_COUNT = 14;         //!< 草
    constexpr int FENCE_COUNT = 14;         //!< 柵
}


class TitleBackground : public IGameObject
{
public:
    /** UpdateSpawner / PreFillScene に渡すオブジェクト種別ごとの設定 */
    struct SpawnConfig {
        float speed          = 100.0f;  //!< スクロール速度
        float spacing        = 300.0f;  //!< 基本間隔
        float minGap         = 150.0f;  //!< 最小間隔
        float baseGap        = 300.0f;  //!< 基準間隔
        float maxGap         = 3.0f;    //!< 最大間隔倍率
        int   maxConsecutive = 8;       //!< 最大連続出現数
        float spawnX         = 1000.0f; //!< 出現X座標
        float z              = 0.0f;    //!< Z座標
        bool  randomRotY     = false;   //!< Y軸回転をランダムにするか
    };

private:
    /** JSON から読み込む全パラメーター。デフォルト値は JSON が見つからない場合のフォールバック */
    struct Param {
        SpawnConfig tree;    //!< 1列目の木
        SpawnConfig tree2;   //!< 2列目の木（z のみ異なる）
        SpawnConfig grass;   //!< 草
        SpawnConfig fence;   //!< 柵
        float groundY        = -30.0f;  //!< 地面のY座標
        float groundScale    = 5000.0f; //!< 地面のスケール
        float cullingMargin  = 100.0f;  //!< 画面外判定のマージン
        float playerX        = -250.0f; //!< プレイヤーX座標
        float playerY        = 0.0f;    //!< プレイヤーY座標
        float playerZ        = 0.0f;    //!< プレイヤーZ座標
        float playerRotYDeg  = 90.0f;   //!< プレイヤーY軸回転角度(度)
        float camPosX        = 0.0f;    //!< カメラ座標X
        float camPosY        = 150.0f;  //!< カメラ座標Y
        float camPosZ        = 800.0f;  //!< カメラ座標Z
        float camTargetX     = 0.0f;    //!< カメラ注視点X
        float camTargetY     = 0.0f;    //!< カメラ注視点Y
        float camTargetZ     = 0.0f;    //!< カメラ注視点Z
        float camFovDeg      = 60.0f;   //!< カメラ視野角(度)
        float camNear        = 1.0f;    //!< ニアクリップ
        float camFar         = 5000.0f; //!< ファークリップ
        float skyCubeScale   = 1000.0f; //!< スカイキューブのスケール
        float playerRunSpeed = 300.0f;  //!< プレイヤーの走行アニメーション速度
    };

    /** スクロールするオブジェクト1体分の状態 */
    struct ScrollModel {
        ModelRender model;                           //!< モデル
        Vector3     position = Vector3::Zero;        //!< 座標
        Quaternion  rotation = Quaternion::Identity; //!< 回転
        bool        active   = false;                //!< スポーン済みで移動中か
        bool        isDraw   = false;                //!< フラスタムカリング結果
    };

    /**
     * スポーナー状態機械
     * Gap  フェーズ：グループ間の間隔を待つ
     * Group フェーズ：グループ内のオブジェクトを順次スポーン
     */
    struct SpawnController {
        /** スポーナーの状態 */
        enum class Phase { Gap, Group };
        Phase phase     = Phase::Gap;   //!< 現在のフェーズ
        int   groupLeft = 0;            //!< グループ内で残りスポーンする数
        float timer     = 0.0f;         //!< 次のイベントまでの残り秒数
    };

private:
    Param   param_; //!< JSONから読み込んだパラメーター

    StaticObject    ground_; //!< 地面
    ScrollModel     trees_  [TitleBGCount::TREE_COUNT];         //!< 1列目の木
    ScrollModel     trees2_ [TitleBGCount::TREE_COUNT_SECOND];  //!< 2列目の木
    ScrollModel     grasses_[TitleBGCount::GRASS_COUNT];        //!< 草
    ScrollModel     fences_ [TitleBGCount::FENCE_COUNT];        //!< 柵

    ModelRender                   playerModel_;     //!< プレイヤーモデル
    AllocatedArray<AnimationClip> playerAnims_;     //!< プレイヤーのアニメーションリスト
    Transform                     playerTransform_; //!< プレイヤーのトランスフォーム

    std::shared_ptr<GameCamera>   titleCamera_; //!< タイトル用カメラ
    SkyCube*                      skyCube_ = nullptr; //!< 天球オブジェクト


    SpawnController treeSpawn_;  //!< 1列目の木のスポーン状態
    SpawnController treeSpawn2_; //!< 2列目の木のスポーン状態
    SpawnController grassSpawn_; //!< 草のスポーン状態
    SpawnController fenceSpawn_; //!< 柵のスポーン状態

    bool scrollEnabled_ = true;   //!< false の間はスポーン・スクロールを停止
    bool playerRunOff_  = false;  //!< true になるとプレイヤーが左へ走り去る
    bool playerGone_    = false;  //!< プレイヤーが画面外に出たら true
    bool visible_       = true;   //!< false の間は Render を全スキップ（Loading 中の負荷軽減）


private:
    /** JSONからパラメーターを読み込む */
    void LoadParam();
    /** カメラの座標・注視点・視野角を反映する */
    void ApplyCamera();
    /** プレイヤーのトランスフォームを反映する */
    void ApplyPlayerTransform();
    /** スポーナーを初期状態にリセットする */
    void ResetSpawners();
    /** 開始時点で画面内をあらかじめオブジェクトで埋めておく */
    void PreFillScene();
    /** 木・草・柵のスポーン処理を進める */
    void UpdateSpawner(SpawnController& ctrl, ScrollModel* objects, int count,
                       const SpawnConfig& cfg);
    /** アクティブなオブジェクトをスクロールさせ、画面外に出たら非アクティブにする */
    void ScrollActiveObjects(ScrollModel* objects, int count, float speed, float despawnX);
    /** フラスタムカリングを行い、描画対象を判定する */
    void UpdateCulling();


public:
    /** コンストラクタ */
    TitleBackground();
    /** デストラクタ */
    ~TitleBackground();

    /** スタート処理 */
    bool Start() override;
    /** 更新処理 */
    void Update() override;
    /** 描画処理 */
    void Render(RenderContext& rc) override;

    /** 木・草・柵のスクロールを止める/再開する */
    inline void SetScrollEnabled(bool enabled) { scrollEnabled_ = enabled; }
    /** プレイヤーを左に走り去らせる */
    inline void StartPlayerRunOff() { playerRunOff_ = true; }
    /** プレイヤーが画面左外に出たか */
    inline bool IsPlayerGone() const { return playerGone_; }
    /** プレイヤー位置・スポーナーを初期状態にリセット（スクロール状態は変えない） */
    void ResetBackground();
    /** 描画をすべてスキップする（Loading 中の負荷軽減用）。SkyCube も連動して非表示にする */
    void SetVisible(bool v)
    {
        visible_ = v;
        if (skyCube_) {
            v ? skyCube_->Activate() : skyCube_->Deactivate();
        }
    }
};
