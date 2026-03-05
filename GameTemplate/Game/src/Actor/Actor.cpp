#include "stdafx.h"
#include "Actor.h"


bool Actor::Start()
{
	return true;
}


void Actor::Update()
{
	// モデルのトランスフォームを更新
	m_transform.UpdateTransform();

	// 更新したトランスフォームをモデルに設定
	m_modelRender.SetPosition(m_transform.m_position);
	m_modelRender.SetScale(m_transform.m_scale);
	m_modelRender.SetRotation(m_transform.m_rotation);
	m_modelRender.Update();
}


void Actor::Render(RenderContext& rc)
{
	m_modelRender.Draw(rc);
}