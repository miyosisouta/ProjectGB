/**
 * OutGameScreen.cpp
 *
 * アウトゲーム画面の描画
 */

#include "stdafx.h"
#include "OutGameScreen.h"


OutGameScreen::OutGameScreen()
{
}


OutGameScreen::~OutGameScreen()
{

}


bool OutGameScreen::Start()
{
	// 画像読み込み
	loadingImage_.Init("Assets/ImageData/.dds", 1920.0f, 1080.0f);

	return true;
}


void OutGameScreen::Update()
{
}


void OutGameScreen::Render(RenderContext& rc)
{
	if (isDraw_) loadingImage_.Draw(rc);
}