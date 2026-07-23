#pragma once

/**
 * StageBase.h
 * ステージごとのギミックがある場合に継承させる予定
 */
#include "src/Stage/StageManager.h"

class StageBase
{
protected:
	/** tklなどを読み込む初期化用関数 */
	virtual void Init() = 0;

public:
	/** コンストラクタ */
	StageBase();
	/** デストラクタ */
	virtual ~StageBase() {};

	/** スタート処理 */
	virtual bool Start() { return true; }
	/** 更新処理 */
	virtual void Update() {}
	/** 描画処理 */
	virtual void Render(RenderContext& rc) {}
};