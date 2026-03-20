/**
 * TitleScreen.h
 *
 * タイトル画面の描画
 */


#pragma once


class TitleScreen : public IGameObject
{
private:
	/** ロード画面の画像 */
	SpriteRender titleImage_;


public:
	TitleScreen();
	~TitleScreen();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};

