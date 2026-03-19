/**
 * TitleScreen.cpp
 *
 * タイトル画面の描画
 */

#include "stdafx.h"
#include "TitleScreen.h"


TitleScreen::TitleScreen()
{
}


TitleScreen::~TitleScreen()
{

}


bool TitleScreen::Start()
{
	// 画像読み込み
	titleImage_.Init("Assets/ImageData/Title.dds", 1920.0f, 1080.0f);

	return true;
}


void TitleScreen::Update()
{
}


void TitleScreen::Render(RenderContext& rc)
{
	titleImage_.Draw(rc);
}