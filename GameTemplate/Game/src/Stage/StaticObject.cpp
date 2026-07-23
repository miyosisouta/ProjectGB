/**
 * StaticObject.cpp
 *
 * ステージの描画を行う
 */


#include "stdafx.h"
#include "StaticObject.h"


void StaticObject::SetupSplatShader(Texture* splatMap, Texture* kusaTex, Texture* tuthiTex, Texture* fuyoudoTex)
{
	// スプラットシェーダー用テクスチャを保持する（Init()内で参照する）
	splatConfig_.splatMap   = splatMap;    // RGBスプラットマップ
	splatConfig_.kusaTex    = kusaTex;     // 草テクスチャ
	splatConfig_.tuthiTex   = tuthiTex;    // 岩土テクスチャ
	splatConfig_.fuyoudoTex = fuyoudoTex;  // 腐葉土テクスチャ
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

	// トランスフォームの初期値を設定（座標・回転・拡縮）
	transform_.localPosition = pos;  // 座標
	transform_.localRotation = rot;  // 回転
	transform_.localScale = scal;    // 拡縮
	transform_.UpdateTransform();

	// 計算済みのワールド座標・回転・拡縮をモデルへ反映
	model_.SetTRS(
		transform_.position,  // 座標
		transform_.rotation,  // 回転
		transform_.scale      // 拡縮
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
