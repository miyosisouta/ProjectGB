/**
 * GameStartMenu.cpp
 * ゲームクリア時の演出等用
 */
#include "stdafx.h"
#include "GameStartMenu.h"
#include "UIAnimationFactory.h"


void GameStartMenu::Update()
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	//taskSchedulerSystem_->Update(deltaTime);
	gamePosSequence_->Update(deltaTime);
	startPosSeauence_->Update(deltaTime);
	obiScaleSeauence_->Update(deltaTime);

	MenuBase::Update();
}


void GameStartMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void GameStartMenu::InitializeLogic()
{
	// ゲームスタートの帯
	auto* backGround = GetUI<UIIcon>(Hash32("gameStart_obi"));
	UIAnimationFactory::Attach<UIScaleAnimation>(backGround, Hash32("obiScaleUp"));
	UIAnimationFactory::Attach<UIScaleAnimation>(backGround, Hash32("obiScaleDown"));
	obiScaleSeauence_ = std::make_unique<UIAnimationSequence>();
	obiScaleSeauence_->Add(Hash32("obiScaleUp")).Add(Hash32("obiScaleDown"), 1.2);
	obiScaleSeauence_->Play(backGround);


	// ゲームの文字を右から左
	auto* game = GetUI<UIIcon>(Hash32("gameStart_game"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(game, Hash32("gamePosA"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(game, Hash32("gamePosB"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(game, Hash32("gamePosC"));
	gamePosSequence_ = std::make_unique<UIAnimationSequence>();
	gamePosSequence_->Add(Hash32("gamePosA")).Add(Hash32("gamePosB")).Add(Hash32("gamePosC"),1.0);
	gamePosSequence_->Play(game);

	// ゲームの文字を右から左
	auto* start = GetUI<UIIcon>(Hash32("gameStart_start"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(start, Hash32("startPosA"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(start, Hash32("startPosB"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(start, Hash32("startPosC"));
	startPosSeauence_ = std::make_unique<UIAnimationSequence>();
	startPosSeauence_->Add(Hash32("startPosA")).Add(Hash32("startPosB")).Add(Hash32("startPosC"), 1.0);
	startPosSeauence_->Play(start);

	//// 演出終わり
	//taskSchedulerSystem_->AddTimer(3.0f, [this]()
	//	{
	//		isEnd_ = true;
	//	});
}


bool GameStartMenu::IsCompletedStartCutSceen()
{
	if (gamePosSequence_->IsPlaying()) {
		return false;
	}
	return true;
}