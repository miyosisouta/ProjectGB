/**
 * LoadingScreen.h
 *
 * ロード画面の処理
 */

#pragma once


class LoadingScreen : public IGameObject
{
private:
	/** ロード画面の画像 */
	SpriteRender loadingImage_;
	/** ロード画面表示のフラグ */
	bool isDraw_ = false;


public:
	/** ロード画面表示のフラグを設定 */
	inline void SetDraw(const bool isDraw) { isDraw_ = isDraw; }


public:
	LoadingScreen();
	~LoadingScreen();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};

