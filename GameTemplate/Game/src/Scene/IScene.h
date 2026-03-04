/**
 * IScene.h
 *
 * 各シーンの基底クラス
 */

#pragma once


 /** 各シーンにIDを割り振る */
#define appScene(scene)\
public:\
 static constexpr uint32_t ID() { return appHash32(#scene); }


class IScene
{
public:
	IScene() {};
	virtual ~IScene() {};


public:
	virtual bool Start() = 0;
	virtual void Update() = 0;
	/** 次のシーンへの切り替え */
	virtual bool RequestScene(uint32_t& id) = 0;
};

