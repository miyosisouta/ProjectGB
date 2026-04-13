#pragma once
/**
 * AttackObject.h
 * 攻撃オブジェクトクラス
 */
class BossCharacter;
class AttackObjectBase
{
protected:
    Transform transform_;
    ModelRender model_; // モデル
    std::unique_ptr<GhostBody> hitbox_; // 当たり判定
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
    void SetupThrow(const Vector3& startPos,
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