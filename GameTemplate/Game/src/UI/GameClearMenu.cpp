/**
 * GameClearMenu.cpp
 * ゲームクリア時の演出等用
 */
#include "stdafx.h"
#include "GameClearMenu.h"
#include "UIAnimationFactory.h"


void GameClearMenu::Update()
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	taskSchedulerSystem_->Update(deltaTime);
	//stampSequence_->Update(deltaTime);
	gameClearSequence_->Update(deltaTime);

	MenuBase::Update();
}


void GameClearMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void GameClearMenu::InitializeLogic()
{
	// ゲームクリアの文字をバウンスする
	auto* gameClearBounce = GetUI<UIIcon>(Hash32("gameClear"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceA"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceB"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceC"));
	gameClearSequence_ = std::make_unique<UIAnimationSequence>();
	gameClearSequence_->Add(Hash32("GameClearStampBounceA")).Add(Hash32("GameClearStampBounceB")).Add(Hash32("GameClearStampBounceC"));


	// 肉球ハンコアイコン
	auto* hankoIcon = GetUI<UIIcon>(Hash32("gameClear_hanko"));
	UIAnimationFactory::Attach<UIColorAnimation>(hankoIcon, Hash32("GameClearStampFadeIn"));
	UIAnimationFactory::Attach<UIScaleAnimation>(hankoIcon, Hash32("GameClearStampScale"));
	stampSequence_ = std::make_unique<UIAnimationSequence>();
	//stampSequence_->Add(Hash32("GameClearStampScale")).Add(Hash32("GameClearStampFadeIn"));
	

	// 演出アニメーションの時間設定
	taskSchedulerSystem_ = std::make_unique<TaskSchedulerSystem>();
	taskSchedulerSystem_->AddTimer(0.0f, [this]()
		{
			auto* gameClearBounce = GetUI<UIIcon>(Hash32("gameClear"));
			gameClearBounce->isDraw = true;
			gameClearSequence_->Play(gameClearBounce);
		});
	taskSchedulerSystem_->AddTimer(0.5f, [this]()
		{
			auto* hankoIcon = GetUI<UIIcon>(Hash32("gameClear_hanko"));
			hankoIcon->isDraw = true;
			hankoIcon->PlayAnimation();
			//stampSequence_->Play(hankoIcon);
		});

	// 演出終わり
	taskSchedulerSystem_->AddTimer(3.0f, [this]()
		{
			isEnd_ = true;
		});
}