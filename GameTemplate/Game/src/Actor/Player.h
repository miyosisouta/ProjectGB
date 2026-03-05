#pragma once
#include "Character.h"


class Player : public Character
{
public:
	/* コンストラクタ */
	Player();
	/* デストラクタ */
	~Player() {}

	/* スタート処理 */
	virtual bool Start() override;
	/* 更新処理 */
	virtual void Update() override;
	/* 描画処理 */
	virtual void Render(RenderContext& rc) override;
};