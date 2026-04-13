#include "stdafx.h"
#include "AttackObject.h"
#include "src/Actor/Character.h"
#include "src/Actor/BossCharacter.h"

namespace {
    /** ThrowRock */
    
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


void ThrowRockObject::SetupThrow(const Vector3& startPos, const Vector3& direction, float collisionSize, float speed, float launchAngle, float gravity)
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
    hitbox_->SetPosition(transform_.position);
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
    hitbox_ = std::make_unique<GhostBody>();
    hitbox_->CreateSphere(
        boss_, 
        CharacterID::BossThrowRockAtkID(),
        collisionSize_,
        ghost::CollisionAttribute::BossAtk, // 自分の属性
        ghost::CollisionAttributeMask::BossAtk // 当たる相手の属性
    );
    hitbox_->SetPosition(transform_.position);

    model_.Update();
	return true;
}

void ThrowRockObject::Update()
{
    Culculate();

    transform_.UpdateTransform();
    model_.SetPosition(transform_.position);
    model_.Update();
    hitbox_->SetPosition(transform_.position);
}
void ThrowRockObject::Render(RenderContext& rc)
{
    model_.Draw(rc);
}