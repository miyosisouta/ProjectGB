/**
 * DebugScene.h
 *
 * デバックの描画
 */

#pragma once
#include "IScene.h"


class StageManagerObject;
class DebugGrassScene : public IScene
{
	Scene(DebugGrassScene);


private:
	StageManagerObject* stage_ = nullptr;

public:
	DebugGrassScene();
	~DebugGrassScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
	bool RequestScene(uint32_t& id) override;
};

