/**
 * IScene.h
 *
 * 各シーンの基底クラス
 */

#pragma once
#include "src/Util/Crc32.h"


 /** 各シーンにIDを割り振る */
#define Scene(scene)\
public:\
 static constexpr uint32_t ID() { return Hash32(#scene); }


class IScene
{
public:
	IScene();
	virtual ~IScene();


public:
	virtual bool Start() = 0;
	virtual void Update() = 0;
	virtual void Render(RenderContext& rc) = 0;;
	/** 次のシーンへの切り替え */
	virtual bool RequestScene(uint32_t& id) = 0;
};

