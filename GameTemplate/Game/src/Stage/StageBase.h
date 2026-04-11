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
	StageBase();
	virtual ~StageBase() {};

	virtual bool Start() { return true; }
	virtual void Update() {}
	virtual void Render(RenderContext& rc) {}
};