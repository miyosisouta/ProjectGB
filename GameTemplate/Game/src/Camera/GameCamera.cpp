/**
 * GameCamera.h
 *
 * インゲームのカメラ処理を行うクラス
 */


#include "stdafx.h"
#include "GameCamera.h"

//todo test
#include "Src/Actor/Player.h"


namespace {
	// カメラ近平面
	constexpr float CAMERA_NEAR = 0.1f;
	// カメラ遠平面
	constexpr float CAMERA_FAR = 10000.0f;
	// カメラの画角
	constexpr float CAMERA_FOVY = 60.0f;
	// 円周率
	constexpr float PI = 3.1415926535f;


	//フリーカメラ関連
	// カメラ回転速度
	constexpr float CAMERA_ROT_SPEED = 0.02f;
	// カメラの高さ
	constexpr float CAMERA_HEIGHT = 150.0f;
	// カメラとプレイヤーの距離
	constexpr float CAMERA_DISTANCE = 400.0f;	

	//ロックオンカメラ関連
	//カメラとプレイヤーの距離
	constexpr float LOCKON_CAMERA_DISTANCE = 200.0f;
	//カメラの高さ
	constexpr float LOCKON_CAMERA_HEIGHT = 100.0f;
}


GameCamera::GameCamera()
{
	// カメラの初期位置を設定
	cameraPosition_ = Vector3(0.0f, CAMERA_HEIGHT, CAMERA_DISTANCE);
	//カメラの注視点を設定
	targetPosition_ = Vector3::Zero;
	//カメラの近平面を設定
	g_camera3D->SetNear(CAMERA_NEAR);
	//カメラの遠平面を設定
	g_camera3D->SetFar(CAMERA_FAR);
	//カメラの画角を設定
	//g_camera3D->SetViewAngle(CAMERA_FOVY);


	//todo test
	player_ = FindGO<Player>("player");
}


GameCamera::~GameCamera()
{
}


void GameCamera::Update()
{
	// カメラのモード管理
	SwitchCameraMode();

	// ゲームカメラの位置を更新
	g_camera3D->SetPosition(cameraPosition_);
	// ゲームカメラの注視点を更新
	g_camera3D->SetTarget(targetPosition_);
}


void GameCamera::SwitchCameraMode()
{
	//ロックオンカメラの切り替え
	if (g_pad[0]->IsTrigger(enButtonRB1)) {
		isLockOn_ = !isLockOn_;
	}


	//状態遷移
	if (isLockOn_) OperateLockOnMode();
	else OperateFreeMode();
}


void GameCamera::OperateFreeMode()
{	
	// RスティックのX値の入力量を取得
	const float rStickX = g_pad[0]->GetRStickXF();

	// 現在のカメラ回転角度を求める
	cameraAngle_ += rStickX * CAMERA_ROT_SPEED;

	const float PI_X2 = 3.14159265f * 2.0f;
	if (cameraAngle_ > PI_X2) cameraAngle_ -= PI_X2;
	if (cameraAngle_ < 0.0f)  cameraAngle_ += PI_X2;

	// カメラ位置を計算
	cameraPosition_.x = player_->GetTransformPosition().x + std::cos(cameraAngle_) * CAMERA_DISTANCE;
	cameraPosition_.y = player_->GetTransformPosition().y + CAMERA_HEIGHT;
	cameraPosition_.z = player_->GetTransformPosition().z + std::sin(cameraAngle_) * CAMERA_DISTANCE;


	//注視点を更新
	targetPosition_ = player_->GetTransformPosition();
}


void GameCamera::OperateLockOnMode()
{
	// 注視点をボスに
	// todo test
	targetPosition_ = Vector3::Zero;

	// カメラの位置はプレイヤーの後ろに
	// プレイヤーからボスへのベクトル
	const Vector3 playerToBoss = targetPosition_ - player_->GetTransformPosition();
	// 反転して正規化
	Vector3 toCamera = playerToBoss * -1;
	toCamera.Normalize();

	// カメラの位置を算出
	cameraPosition_ = player_->GetTransformPosition() + (toCamera * LOCKON_CAMERA_DISTANCE);
	// カメラの高さを調整
	cameraPosition_.y += LOCKON_CAMERA_HEIGHT;
}
