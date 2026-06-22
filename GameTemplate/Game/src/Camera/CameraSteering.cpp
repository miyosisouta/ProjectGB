#include "stdafx.h"
#include "CameraSteering.h"
#include "src/Actor/Character.h"


void CameraSteering::Update(CameraData& data, const float deltaTime)
{
    if (!isUpdate_ || targetCharacter_ == nullptr) {
        return;
    }

    // スティック入力を取得
    const float rawX = g_pad[0]->GetRStickXF(); // Xの入力量
    const float rawY = g_pad[0]->GetRStickYF(); // Yの入力量
    const float invert = invert_ ? -1.0f : 1.0f; // 反転させるか否か
    const float inputX = rawX * sensitivity_ * invert; // 感度と反転を加味したXの入力量
    const float inputY = rawY * sensitivity_ * invert; // 感度と反転を加味したYの入力量

    // 上下の制限
    currentPitch_ -= inputY * config_.rotationSpeedY * deltaTime;
    currentPitch_ = Clamp(currentPitch_, config_.pitchMin, config_.pitchMax);

    // 水平回転を反映
    const float yawDelta = inputX * config_.rotationSpeedX * deltaTime;
    Quaternion yawRot;
    yawRot.SetRotationY(yawDelta);
    yawRot.Apply(toVector_);

    // ピッチを toVector_ の仰角として反映
    const float horizontalDist = config_.distance * cosf(currentPitch_);
    const float verticalOffset = config_.distance * sinf(currentPitch_);
    // toVector_ の XZ 成分は長さ1に正規化して水平距離を掛ける
    Vector3 horizontal = toVector_;
    horizontal.y = 0.0f;
    horizontal.Normalize();
    horizontal *= horizontalDist;

    toVector_ = horizontal;
    toVector_.y = verticalOffset + config_.height;

    // カメラ座標を決定してCameraDataに書き込む
    const Vector3 targetPos = targetCharacter_->transform_.position;
    data.position = targetPos + toVector_;
    data.target = targetPos + Vector3(0.0f, config_.lookAtOffsetY, 0.0f);
}


