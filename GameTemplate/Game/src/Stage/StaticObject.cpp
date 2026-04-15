/**
 * Stage.cpp
 *
 * ステージの描画を行う
 */


#include "stdafx.h"
#include "StaticObject.h"


void StaticObject::Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal)
{
	model_.Init(path);
	transform_.localPosition = pos;
	transform_.localRotation = rot;
	transform_.localScale = scal;
	transform_.UpdateTransform();

	model_.SetTRS(
		transform_.position,
		transform_.rotation,
		transform_.scale
	);
	model_.Update();
}

StaticObject::StaticObject()
{
}
StaticObject::~StaticObject()
{
}

void StaticObject::Render(RenderContext& rc)
{
	if(isDraw_)
	{
		model_.Draw(rc);
	}
}
