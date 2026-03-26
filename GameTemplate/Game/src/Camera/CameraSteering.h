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