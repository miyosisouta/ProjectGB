/**
 * Stage.cpp
 *
 * ステージの描画を行う
 */


#include "stdafx.h"
#include "Stage.h"


Stage::Stage()
{

}


Stage::~Stage()
{
	delete stageCollision_;
	stageCollision_ = nullptr;
}


bool Stage::Start()
{
	// ステージのモデルを読み込む
	modelRender_.Init("Assets/Objects/Stage/Forest/Forest.tkm");

	// ステージの当たり判定用のモデルを読み込む
	collisionModel_.Init("Assets/Objects/Stage/Forest/StageCollision.tkm");
	// コリジョン生成
	stageCollision_ = new PhysicsStaticObject();
	stageCollision_->CreateFromModel(collisionModel_.GetModel(), collisionModel_.GetModel().GetWorldMatrix());

	return true;
}


void Stage::Update()
{
	// 親クラスで更新
	SperClass::Update();
}


void Stage::Render(RenderContext& rc)
{
	// 親クラスで描画
	SperClass::Render(rc);
}