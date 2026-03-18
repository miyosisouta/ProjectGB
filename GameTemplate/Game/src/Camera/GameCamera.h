/**
 * GameCamera.h
 * 
 * インゲームのカメラ処理を行うクラス
 */


#pragma once


class Player;

class GameCamera
{
private:
	/** カメラの位置 */
	Vector3 cameraPosition_ = Vector3::Zero;
	/** 注視点の位置 */
	Vector3 targetPosition_ = Vector3::Zero;
	/** 現在のカメラの回転角度（ラジアン） */
	float cameraAngle_ = 0.0f;
	/** カメラがロックオン状態か */
	bool isLockOn_ = false;

	//todo test
	Player* player_ = nullptr;


public:
	GameCamera();
	~GameCamera();


public:
	void Update();


private:
	/** カメラのモード切替 */
	void SwitchCameraMode();
	/** フリーカメラ操作 */
	void OperateFreeMode();
	/**  ロックオンカメラ操作 */
	void OperateLockOnMode();
};