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
	struct GrassTransform
	{
		Vector3    position = Vector3::Zero;
		Quaternion rotation = Quaternion::Identity;
		Vector3    scale    = Vector3::One;
	};

	StageManagerObject*            stage_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> task_;
	ModelRender                    grassRenderer_;
	std::vector<GrassTransform>    grassTransforms_;

private:
	/* 草をランダムに作成・配置 */
	void GenarateGrass();
	/* jsonに出力 */
	void ExportJson();

public:
	DebugGrassScene();
	~DebugGrassScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
	bool RequestScene(uint32_t& id) override;
};
