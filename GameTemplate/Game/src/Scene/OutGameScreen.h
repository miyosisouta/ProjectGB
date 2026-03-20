/**
 * OutGameScreen.h
 *
 * アウトゲーム画面の描画
 */


#pragma once


class OutGameScreen : public IGameObject
{
private:
	/** アウトゲーム画面の画像 */
	SpriteRender loadingImage_;
	/** アウトゲーム画面表示のフラグ */
	bool isDraw_ = false;


public:
	/** アウトゲーム画面表示のフラグを設定 */
	inline void SetDraw(const bool isDraw) { isDraw_ = isDraw; }


public:
	OutGameScreen();
	~OutGameScreen();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};

