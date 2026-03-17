#include "stdafx.h"
#include "Actor.h"


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
}


void Actor::Render(RenderContext& rc)
{
	modelRender_.Draw(rc);
}