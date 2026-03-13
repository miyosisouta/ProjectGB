/**
 * InGameScene.h
 *
 * インゲームシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"


class InGameScene : public IScene
{
public:
	appScene(InGameScene);


public:
	InGameScene();
	~InGameScene();


public:
	bool Start() override;
	void Update() override;
	bool RequestScene(uint32_t& id) override;
};

