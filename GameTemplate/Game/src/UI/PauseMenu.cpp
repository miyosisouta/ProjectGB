/**
 * PauseMenu.cpp
 * ポーズメニュー
 */
#include "stdafx.h"
#include "PauseMenu.h"
#include "UIAnimationFactory.h"


void PauseMenu::Update()
{
	// Yボタン
	{
		// ボタン押した
		if (g_pad[0]->IsTrigger(enButtonY))
		{
			isReturnTitle_ = true;
		}
	}

	// スタートボタン
	{
		// NOTE: 閉じる処理はInGameSceneの方でしているのでコメントアウト
		//// ボタン押した
		//if (g_pad[0]->IsTrigger(enButtonStart))
		//{
		//	isCloseMenu_ = true;
		//}
	}

	MenuBase::Update();
}


void PauseMenu::Render(RenderContext& rc)
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();


	MenuBase::Render(rc);
}


void PauseMenu::InitializeLogic()
{
}