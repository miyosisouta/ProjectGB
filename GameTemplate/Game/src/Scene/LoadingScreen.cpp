/**
 * LoadingScreen.cpp
 *
 * ロード画面の処理
 */

#include "stdafx.h"
#include "LoadingScreen.h"


namespace
{
	constexpr float FADE_TIME = 1.0f;		//フェードにかかる時間
}


LoadingScreen::LoadingScreen()
{
}


LoadingScreen::~LoadingScreen()
{
}


bool LoadingScreen::Start()
{
	// 画像読み込み
	loadingImage_.Init("Assets/ImageData/Loading.dds", 1920.0f, 1080.0f);

	return true;
}


void LoadingScreen::Update()
{
	// フェードが終了しているなら実行しない
	if (currentFadeTime_ >= FADE_TIME) return;

	// フェード時間を加算
	currentFadeTime_ += g_gameTime->GetFrameDeltaTime();

	// フェードの補完率を算出
	const float fadeRate = currentFadeTime_ / FADE_TIME;

	// フェードイン
	if (isStartDraw_) {

		// フェード終了
		if (fadeRate >= 1.0f) {
			isStartDraw_ = false;
		}

		// フェード続行
		loadingImage_.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, fadeRate));
	}

	// フェードアウト
	if(isEndDraw_) {

		// フェード終了
		if (fadeRate >= 1.0f) {
			isDraw_ = false;
			isEndDraw_ = false;
		}

		// フェード続行
		loadingImage_.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f - fadeRate));
	}
}


void LoadingScreen::Render(RenderContext& rc)
{
	if (isDraw_) {
		loadingImage_.Draw(rc);
	}
}