/**
 * BootScene.h
 *
 * チーム名等の画面の描画
 */

#pragma once
#include "IScene.h"


class Layout;


class BootScene : public IScene
{
	Scene(BootScene);


private:
	Layout* layout_ = nullptr;


public:
	BootScene();
	~BootScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
	bool RequestScene(uint32_t& id) override;
};

