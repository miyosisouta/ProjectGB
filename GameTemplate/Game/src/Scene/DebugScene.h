/**
 * DebugScene.h
 *
 * デバックの描画
 */

#pragma once
#include "IScene.h"
#include "src/Vfx/ParticleEffectRender.h"


class Layout;


class DebugScene : public IScene
{
	Scene(DebugScene);


private:
	Layout* layout_ = nullptr;
	std::vector<std::unique_ptr<ParticleEffectRender>> effectRenderList;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler_ = nullptr;

	std::unique_ptr<GaugeRender> gaugeRender_ = nullptr;

public:
	DebugScene();
	~DebugScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
	bool RequestScene(uint32_t& id) override;
};

