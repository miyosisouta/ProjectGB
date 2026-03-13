/**
 * OutGameScene.h
 *
 * アウトゲームシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"


class OutGameScene : public IScene
{
public:
	appScene(OutGameScene);


public:
	OutGameScene();
	~OutGameScene();


public:
	bool Start() override;
	void Update() override;
	bool RequestScene(uint32_t& id) override;
};

