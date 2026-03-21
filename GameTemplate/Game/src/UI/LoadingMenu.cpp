/**
 * LoadingMenu.cpp
 * サウンドオプションメニュー
 */
#include "stdafx.h"
#include "LoadingMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h"



namespace
{
	//static TaskSchedulerSystem* taskScheduler = nullptr;
}


void LoadingMenu::Update()
{
	taskSchedulerSystem_->Update(g_gameTime->GetFrameDeltaTime());

	MenuBase::Update();
}


void LoadingMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void LoadingMenu::InitializeLogic()
{
	taskSchedulerSystem_ = std::make_unique<TaskSchedulerSystem>();

	const int id = taskSchedulerSystem_->CreateLoopSequence(2.0f);
	const float CHANGE_TIME = 0.5f;

	taskSchedulerSystem_->AddLoopTimer(id, 0.0f, [&]()
		{
			auto* icon = GetUI<UIIcon>(Hash32("loading_run1"));
			icon->isDraw = true;

			icon = GetUI<UIIcon>(Hash32("loading_run4"));
			icon->isDraw = false;
		});

	taskSchedulerSystem_->AddLoopTimer(id, CHANGE_TIME, [&]()
		{
			auto* icon = GetUI<UIIcon>(Hash32("loading_run2"));
			icon->isDraw = true;

			icon = GetUI<UIIcon>(Hash32("loading_run1"));
			icon->isDraw = false;
		});
	taskSchedulerSystem_->AddLoopTimer(id, CHANGE_TIME + CHANGE_TIME, [&]()
		{
			auto* icon = GetUI<UIIcon>(Hash32("loading_run3"));
			icon->isDraw = true;

			icon = GetUI<UIIcon>(Hash32("loading_run2"));
			icon->isDraw = false;
		});
	taskSchedulerSystem_->AddLoopTimer(id, CHANGE_TIME + CHANGE_TIME + CHANGE_TIME, [&]()
		{
			auto* icon = GetUI<UIIcon>(Hash32("loading_run4"));
			icon->isDraw = true;

			icon = GetUI<UIIcon>(Hash32("loading_run3"));
			icon->isDraw = false;
		});
}