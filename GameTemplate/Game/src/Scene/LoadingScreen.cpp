/**
 * LoadingScreen.cpp
 *
 * ロード画面の処理
 */

#include "stdafx.h"
#include "LoadingScreen.h"


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
}


void LoadingScreen::Render(RenderContext& rc)
{
	if (isDraw_) loadingImage_.Draw(rc);
}