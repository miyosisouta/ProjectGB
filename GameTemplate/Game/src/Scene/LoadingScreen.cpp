/**
 * LoadingScreen.cpp
 *
 * ロード画面の処理
 */

#include "stdafx.h"
#include "LoadingScreen.h"
#include "src/UI/Layout.h"
#include "src/UI/LoadingMenu.h"


namespace
{
	constexpr float FADE_TIME = 1.0f;		//フェードにかかる時間
}


LoadingScreen::LoadingScreen()
{
}


LoadingScreen::~LoadingScreen()
{
	delete layout_;
}


bool LoadingScreen::Start()
{
	// 画像読み込み
	//loadingImage_.Init("Assets/ImageData/Loading.dds", 1920.0f, 1080.0f);

	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<LoadingMenu>("Assets/ui/layout/LoadingMenu.json");

	return true;
}


void LoadingScreen::Update()
{
	// フェードが終了しているなら実行しない
	if (currentFadeTime_ >= FADE_TIME) return;

	// フェード時間を加算
	currentFadeTime_ += g_gameTime->GetFrameDeltaTime();

	// フェードの補完率を算出
	float fadeRate = currentFadeTime_ / FADE_TIME;
	if (fadeRate < 0.0f) fadeRate = 0.0f;
	else if (fadeRate > 1.0f) fadeRate = 1.0f;

	// フェードイン
	if (isStartDraw_) {

		// フェード終了
		if (fadeRate >= 1.0f) {
			isStartDraw_ = false;
		}

		// フェード続行
		//loadingImage_.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, fadeRate));
	}

	// フェードアウト
	if(isEndDraw_) {

		// フェード終了
		if (fadeRate >= 1.0f) {
			isDraw_ = false;
			isEndDraw_ = false;
		}

		// フェード続行
		//loadingImage_.SetMulColor(Vector4(0.0f, 0.0f, 0.0f, 1.0f - fadeRate));
	}

	layout_->Update();
}


void LoadingScreen::Render(RenderContext& rc)
{
	//if (isDraw_) loadingImage_.Draw(rc);
	if (isDraw_) {
		layout_->Render(rc);
	}
}