/**
 * CameraSteering.h
 * カメラ操縦処理群
 */
#pragma once
#include "CameraCommon.h"


class Character;


class CameraSteering : public Noncopyable
{
private:
	/** 値の制限 */
	template <typename T>
	inline T Clamp(const T value, const T minVal, const T maxVal)
	{
		if (value < minVal) { return minVal; }
		if (value > maxVal) { return maxVal; }
		return value;
	}


public:
	/** 更新のグループを取得 */
	inline const uint32_t GetUpdateGroup() const { return UpdateGroup::Camera; }

public:
	/** カメラの動作パラメータをまとめた構造体 */
	struct Config
	{
		float distance = 50.0f;						//!< ターゲットとの距離
		float height = 10.0f;						//!< 高さ
		float rotationSpeedX = 1.0f;				//!< X方向の回転速度
		float rotationSpeedY = 1.0f;				//!< Y方向の回転速度

		float pitchMin = Math::DegToRad(-10.0f);	//!< 仰角の下限
		float pitchMax = Math::DegToRad(30.0f);		//!< 仰角の上限
		float lookAtOffsetY = 0.0f;					//!< 注視点のY軸オフセット
	};


private:
	Config config_; //!< カメラの動作パラメータヲまとめた構造体
	Character* targetCharacter_ = nullptr; //!< カメラのターゲット
	Vector3 toVector_ = Vector3::Zero; //!< プレイヤーからカメラへの方向
	float   currentPitch_ = 0.0f; //!< 仰角
	float sensitivity_ = 1.0f; //!< 感度の設定
	bool invert_ = false; //!< 操作の反転するか
	bool isUpdate_ = true; //!< 更新をするか否か

public:
	/** 更新の可否状態を設定 */
	inline void SetUpdate(const bool flg) { isUpdate_ = flg; }


public:
	/** 更新処理 */
	void Update(CameraData& data, const float deltaTime);

	/** 動作パラメータの設定 */
	void SetConfig(const Config& config)
	{
		config_ = config;

		toVector_.z = config_.distance;
		toVector_.y = config_.height;
	}

	/** カメラのターゲットを設定 */
	void SetTargetCharacter(Character* character) { targetCharacter_ = character; }


	/** カメラの上下回転制限設定 */
	void SetLimitDeg(const float minDeg, const float maxDeg)
	{
		config_.pitchMin = Math::DegToRad(minDeg);
		config_.pitchMax = Math::DegToRad(maxDeg);
		currentPitch_ =  Clamp(currentPitch_, config_.pitchMin, config_.pitchMax);
	}

	/** カメラの操作を反転させるかの設定 */
	void SetInvert(const bool invert) { invert_ = invert; }
	/** カメラの操作感度の設定 */
	void SetSensitivity(const float value) { sensitivity_ = value; }
	/** カメラとターゲットとの距離の設定 */
	void SetDistance(const float value) { config_.distance = value; }

	/** カメラの操作を反転させるかの取得 */
	bool GetInvert() const { return invert_; }
	/** カメラの操作感度の取得 */
	float GetSensitivity() const { return sensitivity_; }
	/** カメラとターゲットとの距離の取得 */
	float GetDistance()    const { return config_.distance; }
};