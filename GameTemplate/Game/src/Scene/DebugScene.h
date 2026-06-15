/**
 * DebugScene.h
 *
 * デバックの描画
 */

#pragma once
#include "IScene.h"
#include "src/UI/Menu.h"
//#include "src/Util/TaskSchedulerSystem.h"


class Layout;


class DebugScene : public IScene
{
	Scene(DebugScene);


private:
	Layout* layout_ = nullptr;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler_ = nullptr;

public:
	DebugScene();
	~DebugScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
	bool RequestScene(uint32_t& id) override;
};

