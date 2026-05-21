#include "stdafx.h"
#include "CameraController.h"
#include "CameraManager.h"


#ifdef _DEBUG

void DebugCamera::OnEnter()
{
	cameraData_ = CameraManager::Get().GetCurrentCameraData();
}


void DebugCamera::Update()
{
	// fov調整
	if (g_pad[0]->IsPress(enButtonRB1)) {
		float value = g_pad[0]->GetLStickYF();
		cameraData_.fov += value * 0.05f;
		return;
	}
	// 左スティックで移動
	{
		Vector3 inputDirection;
		inputDirection.x = g_pad[0]->GetLStickXF();
		inputDirection.z = g_pad[0]->GetLStickYF();

		// カメラの前方向と右方向のベクトルを取得
		Vector3 forward = g_camera3D->GetForward();
		Vector3 right = g_camera3D->GetRight();

		// y方向には移動しない
		forward.y = 0.0f;
		right.y = 0.0f;

		// 左スティックの入力量を加算
		right *= inputDirection.x;
		forward *= inputDirection.z;

		Vector3 direction = right + forward;
		direction.Normalize();
		// 移動速度調整
		direction.Scale(10.0f);

		// 平行移動
		cameraData_.position += direction;
		cameraData_.target += direction;
	}
	
	// カメラの上下
	{
		float up = 0.0f;
		if (g_pad[0]->IsPress(enButtonDown)) up = -1.0f;  // 下
		if (g_pad[0]->IsPress(enButtonUp)) up = 1.0f;  // 上（好みのボタンに変更）
		// または右スティック押し込みなど

		Vector3 upMove(0.0f, up * 10.0f, 0.0f);
		cameraData_.position += upMove;
		cameraData_.target += upMove;
	}

	// 右スティックで回転
	{
		float rotX = g_pad[0]->GetRStickXF() * 0.05f;
		float rotY = g_pad[0]->GetRStickYF() * 0.05f;

		// rotXでY軸回転
		Quaternion yRotation;
		yRotation.SetRotationY(-rotX);
		Vector3 toVector = cameraData_.position - cameraData_.target;
		yRotation.Apply(toVector);
		// rotYでXZ軸回転
		Quaternion xzRotation;
		xzRotation.SetRotation(g_camera3D->GetRight(), -rotY);
		xzRotation.Apply(toVector);
		cameraData_.position = cameraData_.target + toVector;
	}
}
#endif