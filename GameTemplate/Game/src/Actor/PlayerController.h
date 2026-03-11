#pragma once
class Player;
class PlayerController : public IGameObject
{
public:
	/* コンストラクタ */
	PlayerController();
	/* デストラクタ */
	~PlayerController();

	/* スタート処理 */
	bool Start()override;
	/* 更新処理 */
	void Update()override;
	/* 描画処理 : 今回は使わない */
	void Render(RenderContext& rc) {}


};

