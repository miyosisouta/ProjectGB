#include "stdafx.h"
#include "Actor.h"
#include "ActorStatus.h"


bool Actor::Start()
{
	return true;
}


void Actor::Update()
{
	// モデルのトランスフォームを更新
	transform_.UpdateTransform();

	// 更新したトランスフォームをモデルに設定
	modelRender_.SetPosition(transform_.position);
	modelRender_.SetScale(transform_.scale);
	modelRender_.SetRotation(transform_.rotation);
	modelRender_.Update();

	// ステータスの更新
	if (status_) { status_->Update(); }
}


void Actor::Render(RenderContext& rc)
{
	// 非表示設定中は描画処理をスキップ
	if (!isDraw_) { return; } 

	// 描画
	modelRender_.Draw(rc);
}