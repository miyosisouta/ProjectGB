#pragma once
/**
 * AttackObject.h
 * 実体のある攻撃オブジェクトクラス
 */
class Character;
class Player;
class BossCharacter;
class AttackRange;
class AttackObjectBase
{
protected:
    std::unique_ptr<TaskSchedulerSystem> taskScheduler_; //!< タイマー付きタスクの管理
    Transform transform_; //!< トランスフォーム
    ModelRender model_; //!< モデル
    std::unique_ptr<GhostBody> attackHitBox_; //!< 当たり判定
    bool isFinished_ = false; //!< 処理が終わったかどうか

public:
    /** モデルを取得 */
    inline ModelRender* GetModelRender() { return &model_; }
    /** 処理が終わったか */
    inline bool IsFinished() { return isFinished_; }

    /** 地面に着地しているか */
    bool OnGround();


public:
    /** コンストラクタ */
    AttackObjectBase();
    /** デストラクタ */
    virtual ~AttackObjectBase();

public:
    /** スタート処理 */
    virtual bool Start() = 0;
    /** 更新処理 */
    virtual void Update() = 0;
    /** 描画処理 */
    virtual void Render(RenderContext& rc) = 0;
};


/**
 * 岩を投げる攻撃
 */
class ThrowRockObject : public AttackObjectBase
{
private:
    BossCharacter* boss_ = nullptr; //!< 投げたボス
    /** 岩が飛ぶ際に使用するパラメータ */
    Vector3 flatVelocity_     = Vector3::Zero; //!< 水平方向の速度
    float   collisionSize_    = 0.0f;          //!< コリジョンのサイズ
    float   verticalVelocity_ = 0.0f;          //!< 垂直方向の速度
    float   gravity_          = 0.0f;          //!< 重力加速度
    float   flySpeed_         = 0.0f;          //!< 水平飛翔スピード
    float   launchAngle_      = 0.0f;          //!< 投射初速度の仰角
    float   traveledDist_     = 0.0f;          //!< 飛んだ距離の累計

    /** ワインドアップ中にスケール・座標を変化させながら見せる演出用パラメータ */
    bool    isLaunched_     = false;                        //!< Launch()が呼ばれた（投げられた）か
    Vector3 startScale_     = Vector3(0.1f, 0.1f, 0.1f);    //!< 出現直後のスケール
    Vector3 targetScale_    = Vector3::One;                 //!< 投げられる瞬間に到達させる最終スケール（Launch()時のサイズと必ず一致する）
    Vector3 growStartPos_   = Vector3::Zero;                //!< 出現直後の座標（Setup()で渡されたstartPosのYだけ低くしたもの）
    Vector3 growTargetPos_  = Vector3::Zero;                //!< 投げる瞬間の座標（Setup()で渡されたstartPosそのもの）


public:
    /** 投げる際のセットアップ（この時点ではまだ飛ばない。Launch()を呼ぶまでスケール・座標はSetGrowProgress()で外部から制御する） */
    void Setup(const Vector3& startPos,
        const Vector3& direction,
        float collisionSize,
        float speed,
        float launchAngle,
        float gravity,
        float spawnHeight);

    /** 投げたボスを設定 */
    inline void SetBossData(BossCharacter* boss) { boss_ = boss; }
    /**
     * 出現(Setup)からLaunch()までのスケール・座標の変化具合を呼び出し側(ThrowRockState)から毎フレーム設定する。
     * t=0で出現直後のサイズ・座標、t=1で投げる瞬間のサイズ(targetScale_)・座標(growTargetPos_)になる。
     * 予測ラインの表示進捗と同じ進捗値を渡すことで、投げる瞬間の見た目と必ず一致するようにする。
     */
    void SetGrowProgress(float t);
    /** 育てていた岩を実際に投げる（ここで初めて移動・当たり判定が有効になる。まだ成長しきっていなくても最終スケール・座標に揃える） */
    void Launch();


private:
    /** 岩の移動計算 */
    void Culculate();

public:
    /** コンストラクタ */
    ThrowRockObject();
    /** デストラクタ */
    ~ThrowRockObject();

    /** スタート処理 */
    bool Start()override;
    /** 更新処理 */
    void Update()override;
    /** 描画処理 */
    void Render(RenderContext& rc)override;
};


/**
 * 地雷攻撃
 */
class LandmineObject : public AttackObjectBase
{
private:
    /** 地雷の進行状態 */
    enum class Phase
    {
        enWaiting, //!< 設置直後の待機中
        enWarning, //!< 爆発予測を表示中
        enExpload, //!< 爆発済み
        enDone     //!< 一連の処理が終わったか
    };

private:
    EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE; //!< 爆発エフェクトのハンドル
    AttackRange* attackRangeIndicator_ = nullptr; //!< 爆発予測サークルインジケーター
    Phase phase_; //!< 状態
    Character* owner_ = nullptr; //!< 設置したキャラクター
    float motionValue_ = 0.0f; //!< 威力

public:
    /** 設置したキャラクターを設定 */
    inline void SetOwnerData(Character* character) { owner_ = character; }
    /** セットアップ */
    void Setup(Vector3 targetPos, float motionValue);

public:
    /** コンストラクタ */
    LandmineObject();
    /** デストラクタ */
    ~LandmineObject();

    /** スタート処理 */
    bool Start()override;
    /** 更新処理 */
    void Update()override;
    /** 描画処理 */
    void Render(RenderContext& rc)override;
};