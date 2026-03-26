/**
 * CameraSteering.h
 * カメラ操縦処理群
 */
#pragma once
#include "CameraCommon.h"


class Character;


class CameraSteering : public Noncopyable
{
public:
	/** 更新のグループを取得 */
	inline const uint32_t GetUpdateGroup() const { return UpdateGroup::CAMERA; }

public:
	struct Config
	{
		float distance = 50.0f;
		float height = 10.0f;
		float rotationSpeedX = 1.0f;
		float rotationSpeedY = 1.0f;
	};


private:
	Config config_;
	Character* targetCharacter_ = nullptr;
	Vector3 toVector_ = Vector3::Zero;
	bool isUpdate_ = true;


public:
	/** 更新の可否状態を設定 */
	inline void SetUpdate(const bool flg) { isUpdate_ = flg; }


public:
	void Update(CameraData& data, const float deltaTime);

	void SetConfig(const Config& config)
	{
		config_ = config;

		toVector_.z = config_.distance;
		toVector_.y = config_.height;
	}
	void SetTargetCharacter(Character* character) { targetCharacter_ = character; }
};