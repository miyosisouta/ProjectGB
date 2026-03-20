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
	/** 現在のフェード時間 */
	float currentFadeTime_ = 0.0f;
	/** ロード画面表示のフラグ */
	bool isDraw_ = false;
	/** ロード画面描画開始のフラグ */
	bool isStartDraw_ = false;
	/** ロード画面描画終了のフラグ */
	bool isEndDraw_ = false;


public:
	/** ロード画面の表示を開始する */
	inline void StartDraw() { 
		isStartDraw_ = true; 
		isDraw_ = true;
		currentFadeTime_ = 0.0f;
	};

	/** ロード画面の表示を終了する */
	void EndDraw() { 
		isEndDraw_ = true;
		currentFadeTime_ = 0.0f;
	}


public:
	LoadingScreen();
	~LoadingScreen();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc) override;
};

