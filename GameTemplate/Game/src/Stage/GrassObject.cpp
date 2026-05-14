/**
 * Stage.cpp
 *
 * ステージの描画を行う
 */


#include "stdafx.h"
#include "GrassObject.h"


void GrassObject::Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal)
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

	SetDitherAlpha(1.0f);
}


GrassObject::GrassObject()
{
}
GrassObject::~GrassObject()
{
}

void GrassObject::Update()
{
	transform_.UpdateTransform();

	model_.SetTRS(
		transform_.position,
		transform_.rotation,
		transform_.scale
	);
	model_.Update();
}
void GrassObject::Render(RenderContext& rc)
{
	if(isDraw_)
	{
		model_.Draw(rc);
	}
}
