#include "stdafx.h"
#include "CameraSteering.h"
#include "src/Actor/Character.h"


namespace
{
}


void CameraSteering::Update(CameraData& data, const float deltaTime)
{
	// カメラがアクティブでない場合は更新しない
	if (!isUpdate_) return;


	if (targetCharacter_ == nullptr) {
		return;
	}
	CameraData nextData = data;


	// 理想のカメラ位置を計算（ターゲットの後ろ・上）
	// ※簡易的にZ軸手前に引いていますが、本来はターゲットの向き(Rotation)も考慮して回転させます
	Vector3 targetPosition = targetCharacter_->transform_.position;
	Vector3 position = targetCharacter_->transform_.position + toVector_;

	nextData.position = position;
	nextData.target = targetPosition;

	// 右スティックで回転
	Vector3 rotationVector = Vector3(g_pad[0]->GetRStickXF(), g_pad[0]->GetRStickYF(), 0.0f);
	if (rotationVector.LengthSq() > 0.001f) {
		rotationVector.x *= config_.rotationSpeedX;
		rotationVector.y *= config_.rotationSpeedY;
		// 先に上下回転
		Quaternion xzRotation;
		xzRotation.SetRotation(g_camera3D->GetRight(), rotationVector.y);
		xzRotation.Apply(toVector_);

		// 後に左右回転
		Quaternion yRotation;
		yRotation.SetRotationY(-rotationVector.x);
		yRotation.Apply(toVector_);
		nextData.position = nextData.target + toVector_;
	}

	data = nextData;
}