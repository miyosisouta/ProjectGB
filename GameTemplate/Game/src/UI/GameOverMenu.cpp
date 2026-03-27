/**
 * GameOverMenu.cpp
 * ゲームオーバー時の演出等用
 */
#include "stdafx.h"
#include "GameOverMenu.h"
#include "UIAnimationFactory.h"


void GameOverMenu::Update()
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	taskSchedulerSystem_->Update(deltaTime);
	//stampSequence_->Update(deltaTime);
	gameOverSequence_->Update(deltaTime);

	MenuBase::Update();
}


void GameOverMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void GameOverMenu::InitializeLogic()
{
	// ゲームオーバーの背景
	auto* gameOverFade = GetUI<UIIcon>(Hash32("gameOverFade"));
	UIAnimationFactory::Attach<UIColorAnimation>(gameOverFade, Hash32("GameOverFade"));
	//gameOverStampSequence_ = std::make_unique<UIAnimationSequence>();

	// ゲームオーバーの文字移動(上から下)
	auto* gameOverDown= GetUI<UIIcon>(Hash32("gameOver"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameOverDown, Hash32("GameOverTranslateOffset"));
	UIAnimationFactory::Attach<UIColorAnimation>(gameOverDown, Hash32("GameOverFadeIn"));
	gameOverSequence_ = std::make_unique<UIAnimationSequence>();

	// 落ち込むアイコン
	auto* otikomuIcon = GetUI<UIIcon>(Hash32("gameOver_otikomu"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(otikomuIcon, Hash32("GameOver_otikomu_TranslateOffset"));
	UIAnimationFactory::Attach<UIColorAnimation>(otikomuIcon, Hash32("GameOver_otikomu_FadeIn"));
	gameOverStampSequence_ = std::make_unique<UIAnimationSequence>();


	// 演出アニメーションの時間設定
	taskSchedulerSystem_ = std::make_unique<TaskSchedulerSystem>();
	taskSchedulerSystem_->AddTimer(0.0f, [this]()
		{
			auto* gameOverFade = GetUI<UIIcon>(Hash32("gameOverFade"));
			gameOverFade->isDraw = true;
			gameOverFade->PlayAnimation();
		});
	taskSchedulerSystem_->AddTimer(0.4f, [this]()
		{
			auto* gameOverDown = GetUI<UIIcon>(Hash32("gameOver"));
			auto* otikomuIcon = GetUI<UIIcon>(Hash32("gameOver_otikomu"));
			gameOverDown->isDraw = true;
			otikomuIcon->isDraw = true;
			gameOverSequence_->Play(gameOverDown);
			gameOverStampSequence_->Play(otikomuIcon);
		});
	// 演出終わり
	taskSchedulerSystem_->AddTimer(3.0f, [this]()
		{
			isEnd_ = true;
		});
}