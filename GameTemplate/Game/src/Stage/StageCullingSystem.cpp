/**
 * StageCullingSystem.cpp
 *
 * ステージカリングのシステム
 */


#include "stdafx.h"
#include "StageCullingSystem.h"

#include "StaticObject.h"
#include "src/Collision/BoundingVolume.h"


StageCullingSystem::StageCullingSystem()
{
}


StageCullingSystem::~StageCullingSystem()
{
}


void StageCullingSystem::Update(std::vector<StaticObject*> staticObjects)
{
	// --- VP 行列からフラスタムを構築（毎フレーム更新） ---
	Frustum frustum;
	frustum.BuildFromViewProjectionMatrix(g_camera3D->GetViewProjectionMatrix());

	for (auto* object : staticObjects) {
		// モデルの頂点から AABB を計算
		Bounds bounds;
		bounds.Compute(object->GetModel()->GetModel());

		bounds.maxPoint += object->GetTransform()->position;
		bounds.minPoint += object->GetTransform()->position;

		// フラスタム内かどうか判定して描画フラグを切り替える
		if (frustum.IsVisible(bounds)) {
			object->SetDraw(true);   // 描画する
		}
		else {
			object->SetDraw(false);  // カリング（スキップ）
		}
	}
}



