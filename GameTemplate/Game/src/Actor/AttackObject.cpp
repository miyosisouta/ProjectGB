#include "stdafx.h"
#include "AttackObject.h"
#include "src/Actor/Character.h"
#include "src/Actor/BossCharacter.h"
#include "src/Stage/GrassBendManager.h"

namespace {
    /** Landmine */
    static Vector3 MODEL_LANDMINE_SCALE = Vector3(0.1f, 0.1f, 0.1f); // モデルの大きさ
    constexpr float LANDMINE_COLLISION_SIZE = 350.0f; // 爆発半径
    constexpr float EFFECT_SCALE_FACTOR_DAMAGE_LING = 0.5f;
    constexpr float EFFECT_SCALE_FACTOR_EXPLOAD = 0.15f;
}
/*=======================================*/
/* 攻撃用オブジェクトの基底クラス */
/*=======================================*/

bool AttackObjectBase::OnGround()
{
    if (transform_.position.y <= 0.0f) {
        return true;
    }
    return false;
}

AttackObjectBase::AttackObjectBase()
{
}
AttackObjectBase::~AttackObjectBase()
{
}

bool AttackObjectBase::Start()
{
    return true;
}

void AttackObjectBase::Update()
{
}

void AttackObjectBase::Render(RenderContext& rc)
{
}


/*=======================================*/
/* 岩を投げる攻撃 */
/*=======================================*/


void ThrowRockObject::Setup(const Vector3& startPos, const Vector3& direction, float collisionSize, float speed, float launchAngle, float gravity)
{
    transform_.localPosition = startPos;
    collisionSize_ = collisionSize;
    flySpeed_ = speed;
    launchAngle_ = launchAngle;
    gravity_ = -gravity; // 内部では負の値として持つ（下向きに働くため）
    transform_.UpdateTransform();

    // 水平方向の速度ベクトル
    float rad = Math::DegToRad(launchAngle_); // 角度計算
    float hSpeed = speed * cos(rad); // 速度を計算
    flatVelocity_ = direction * hSpeed; // 飛ぶ方向と速度を計算

    // 垂直方向の初速（仰角から計算）
    verticalVelocity_ = speed * sin(rad);
}

void ThrowRockObject::Culculate()
{
    if (OnGround()) { isFinished_ = true; return; } // 岩が壊れる処理をここに書く

    float time = g_gameTime->GetFrameDeltaTime();

    verticalVelocity_ += gravity_ * time; // gravity_ は負なので減算になる

    Vector3 moveThisFrame = flatVelocity_ * time;
    moveThisFrame.y += verticalVelocity_ * time;

    transform_.localPosition += moveThisFrame;
    transform_.UpdateTransform();

    // 飛んだ距離を累計（水平成分のみ）
    traveledDist_ += Vector3(moveThisFrame.x, 0.0f, moveThisFrame.z).Length();

    // モデルとコリジョンの座標を同期
    model_.SetPosition(transform_.position);
    model_.Update();
    attackHitBox_->SetPosition(transform_.position);
}



ThrowRockObject::ThrowRockObject()
{
}

ThrowRockObject::~ThrowRockObject()
{
}

bool ThrowRockObject::Start()
{
    // モデルの読み込み
    model_.Init("Assets/Objects/Skill/ThrowRock/throwRock.tkm");
    model_.SetPosition(transform_.position);

    // コリジョンの作成
    attackHitBox_ = std::make_unique<GhostBody>();
    attackHitBox_->CreateSphere(
        boss_,
        CharacterID::BossThrowRockAtkID(),
        collisionSize_,
        ghost::CollisionAttribute::BossAtk, // 自分の属性
        ghost::CollisionAttributeMask::BossAtk // 当たる相手の属性
    );
    attackHitBox_->SetPosition(transform_.position);

    // 草を曲げる
    if (GrassBendManager::IsInitialized())
    {
        if (const auto* gp = ParameterManager::Get().GetGrassBendParam("ThrowRock"))
        {
            GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
            GrassBendManager::Get().AddSource(transform_.position, params);
        }
    }

    model_.Update();
	return true;
}

void ThrowRockObject::Update()
{
    Culculate();

    transform_.UpdateTransform();
    model_.SetPosition(transform_.position);
    model_.Update();
    attackHitBox_->SetPosition(transform_.position);
}
void ThrowRockObject::Render(RenderContext& rc)
{
    model_.Draw(rc);
}



/*=======================================*/
/* 地雷攻撃 */
/*=======================================*/

void LandmineObject::Setup(Vector3 targetPos, float motionValue)
{
    motionValue_ = motionValue;
    transform_.localPosition = targetPos;
    transform_.localScale = MODEL_LANDMINE_SCALE;
    transform_.UpdateTransform();

    predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
    phase_ = Phase::enWaiting;
}

LandmineObject::LandmineObject()
{
}

LandmineObject::~LandmineObject()
{
}

bool LandmineObject::Start()
{
    // モデルの読み込み
    model_.Init("Assets/Objects/Skill/Landmine/GameData/Landmine.tkm");
    model_.SetPosition(transform_.position);
    model_.SetScale(transform_.scale);
    model_.Update();

    // エフェクトサイズ
    Vector3 damageZoneEffectScale = LANDMINE_COLLISION_SIZE * EFFECT_SCALE_FACTOR_DAMAGE_LING;
    Vector3 exploadEffectScale = LANDMINE_COLLISION_SIZE * EFFECT_SCALE_FACTOR_EXPLOAD;
    float collisionScale = LANDMINE_COLLISION_SIZE;


    // タスクスケジューラーを作成
    taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

    // 攻撃範囲の可視化
    taskScheduler_->AddTimer(3.0f, [this, damageZoneEffectScale]() {
        predictionEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_DamageZone_Ring,
            transform_.position,
            Quaternion::Identity,
            damageZoneEffectScale
        );

        phase_ = Phase::enWarning;
        });


    // 爆発
    taskScheduler_->AddTimer(6.0f, [this, exploadEffectScale,collisionScale]() {
        // 攻撃予測エフェクトを停止
        if (predictionEffectHandle_ != INVALID_EFFECT_HANDLE) {
            EffectManager::Get().StopEffect(predictionEffectHandle_);
            predictionEffectHandle_ = INVALID_EFFECT_HANDLE;
        }

        // 爆発エフェクトを再生
        predictionEffectHandle_ = EffectManager::Get().PlayEffect(
            enEffectKind_Expload,
            transform_.position,
            Quaternion::Identity,
            exploadEffectScale
        );

        SoundManager::Get().PlaySE(enSoundKind_Player_Landmine);
        attackHitBox_ = std::make_unique<GhostBody>();
        attackHitBox_->CreateSphere(
            owner_,
            CharacterID::CharaLandmineAtkID(),
            collisionScale,
            ghost::CollisionAttribute::CharacterAtk, // キャラクター誰でも攻撃可能
            ghost::CollisionAttributeMask::CharacterAtk // キャラクター全員ダメージを受ける
        );
        attackHitBox_->SetPosition(transform_.position);

        // 草を曲げる
        if (GrassBendManager::IsInitialized())
        {
            if (const auto* gp = ParameterManager::Get().GetGrassBendParam("Landmine"))
            {
                GrassBendManager::AttackParams params{ gp->force, gp->radius, gp->duration, gp->recoverySpeed };
                GrassBendManager::Get().AddSource(transform_.position, params);
            }
        }

        phase_ = Phase::enExpload;
        });

    // 削除
    taskScheduler_->AddTimer(6.1f, [&]() {
        attackHitBox_.reset();
        phase_ = Phase::enDone;
        });
    return true;
}

void LandmineObject::Update()
{
    // タスクスケジューラーの時間を更新
    if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }

    // 処理が終了したら
    if (phase_ == Phase::enDone) 
    {
        isFinished_ = true;
    }

    model_.Update();
}

void LandmineObject::Render(RenderContext& rc)
{
    model_.Draw(rc);
}
