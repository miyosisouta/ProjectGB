/**
 * Stage.h
 *
 * ステージの描画を行う
 */


#pragma once
#include "src/Actor/Actor.h"


class Stage : public Actor
{
	/** 親クラスはActor */
	using SperClass = Actor;


private:
	/** 当たり判定用のモデル */
	ModelRender collisionModel_;
	/** ステージのコリジョン */
	PhysicsStaticObject* stageCollision_ = nullptr;


public:
	Stage();
	~Stage();

	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};