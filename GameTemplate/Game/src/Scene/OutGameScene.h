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
	Scene(OutGameScene);


public:
	OutGameScene();
	~OutGameScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
	bool RequestScene(uint32_t& id) override;
};

