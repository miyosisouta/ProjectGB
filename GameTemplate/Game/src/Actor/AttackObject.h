#pragma once
/**
 * AttackObject.h
 * 攻撃オブジェクトクラス
 */
class Character;
class Player;
class BossCharacter;
class AttackRange;
class AttackObjectBase
{
protected:
    std::unique_ptr<TaskSchedulerSystem> taskScheduler_;
    Transform transform_;
    ModelRender model_; // モデル
    std::unique_ptr<GhostBody> attackHitBox_; // 当たり判定
    bool isFinished_ = false; // 処理が終わったかどうか

public:
    /* モデルを取得 */
    ModelRender* GetModelRender() { return &model_; }
    bool IsFinished() { return isFinished_; }

    bool OnGround();


public:
    AttackObjectBase();
    virtual ~AttackObjectBase();

public:
    virtual bool Start() = 0;
    virtual void Update() = 0;
    virtual void Render(RenderContext& rc) = 0;
};


/*
 * 岩を投げる攻撃
 */
class ThrowRockObject : public AttackObjectBase
{
private:
    BossCharacter* boss_ = nullptr;
    /** 岩が飛ぶ際に使用するパラメータ */
    Vector3 flatVelocity_ = Vector3::Zero; // 水平方向の速度
    float collisionSize_ = 0.0f; // コリジョンのサイズ
    float verticalVelocity_ = 0.0f; // 垂直方向の速度
    float gravity_ = 0.0f; // 重力加速度
    float flySpeed_ = 0.0f; // 水平飛翔スピード
    float launchAngle_ = 0.0f; // 投射初速度の仰角
    float traveledDist_ = 0.0f; // 飛んだ距離の累計

public:
    /** 投げる際のセットアップ */
    void Setup(const Vector3& startPos,
        const Vector3& direction,
        float collisionSize,
        float speed,
        float launchAngle,
        float gravity);

    void SetBossData(BossCharacter* boss) { boss_ = boss; }
private:
    /** 岩の移動計算 */
    void Culculate();

public:
    ThrowRockObject();
    ~ThrowRockObject();

    bool Start()override;
    void Update()override;
    void Render(RenderContext& rc)override;
};


/*
 * 地雷攻撃
 */
class LandmineObject : public AttackObjectBase
{
private:
    enum class Phase
    {
        enWaiting,
        enWarning,
        enExpload,
        enDone
    };

private:
    EffectHandle predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
    AttackRange* attackRangeIndicator_ = nullptr; //!< 爆発予測サークルインジケーター
    Phase phase_; // 状態
    Character* owner_ = nullptr; //!< 設置したキャラクター
    float motionValue_ = 0.0f; //!< 威力

public:
    /** 設置したキャラクターを設定 */
    void SetOwnerData(Character* character) { owner_ = character; }
    /** セットアップ */
    void Setup(Vector3 targetPos, float motionValue);

public:
    LandmineObject();
    ~LandmineObject();

    bool Start()override;
    void Update()override;
    void Render(RenderContext& rc)override;
};