/**
 * Stage.cpp
 *
 * ステージの描画を行う
 */


#include "stdafx.h"
#include "StaticObject.h"


void StaticObject::SetupSplatShader(Texture* splatMap, Texture* kusaTex, Texture* tuthiTex, Texture* fuyoudoTex)
{
	splatConfig_.splatMap   = splatMap;
	splatConfig_.kusaTex    = kusaTex;
	splatConfig_.tuthiTex   = tuthiTex;
	splatConfig_.fuyoudoTex = fuyoudoTex;
}

void StaticObject::Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal)
{
	// スプラットシェーダーが設定されている場合、Init() 前にオーバーライドをセット
	if (splatConfig_.isValid()) {
		model_.SetGBufferFxOverride("Assets/shader/ground_splat.fx");
		// t11-t14 にスプラットテクスチャをバインド (index=0→t11, 1→t12, 2→t13, 3→t14)
		model_.SetExtraGBufferTextureSRV(0, splatConfig_.splatMap);
		model_.SetExtraGBufferTextureSRV(1, splatConfig_.kusaTex);
		model_.SetExtraGBufferTextureSRV(2, splatConfig_.tuthiTex);
		model_.SetExtraGBufferTextureSRV(3, splatConfig_.fuyoudoTex);
	}
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
