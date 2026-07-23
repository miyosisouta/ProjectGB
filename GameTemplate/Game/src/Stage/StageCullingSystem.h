/**
 * StageCullingSystem.h
 * ステージカリングのシステム
 */
#pragma once


class StaticObject;


class StageCullingSystem
{
public:
	/** コンストラクタ */
	StageCullingSystem();
	/** デストラクタ */
	~StageCullingSystem();

	/** フラスタムカリングを行い、各オブジェクトの描画フラグを更新する */
	void Update(std::vector<StaticObject*> staticObjects);
};