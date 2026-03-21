/**
 * InGameScene.h
 *
 * インゲームシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"

class Layout;

class InGameScene : public IScene
{
private:
	Layout* layout_ = nullptr;

public:
	Scene(InGameScene);


public:
	InGameScene();
	~InGameScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc);
	bool RequestScene(uint32_t& id) override;
};

