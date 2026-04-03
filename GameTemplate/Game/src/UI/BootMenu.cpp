/**
 * BootMenu.cpp
 * ブートメニュー
 */
#include "stdafx.h"
#include "BootMenu.h"
#include "UIAnimationFactory.h"
#include "Layout.h"

namespace
{
	constexpr float SHORT_WAIT_TIME = 2.0f;
	constexpr float MAX_WAIT_TIME = 6.0f;
}


void BootMenu::Update()
{
	switch (step_)
	{
		case PublisherLogoStart:
		{
			step_ = PublisherLogoUpdate;
			/** FALL THROUGH */
		}
		case PublisherLogoUpdate:
		{
			// 指定した時間経過した
			if (elapsedTime_ >= MAX_WAIT_TIME) {
				step_ = TeamLogoStart;
			}
			if (elapsedTime_ >= SHORT_WAIT_TIME && g_pad[0]->IsTrigger(enButtonA)) {
				bootImageSequence_->Clear();
				step_ = TeamLogoStart;
			}
			break;
		}
		case TeamLogoStart:
		{
			elapsedTime_ = 0.0f;

			auto* startUpKBC = GetUI<UIIcon>(Hash32("kbcTitle"));
			startUpKBC->color.w = 0.0f;

			auto* teamNameTitle = GetUI<UIIcon>(Hash32("teamNameTitle"));
			bootImageSequence_->Add(Hash32("boot_Fadein")).Add(Hash32("boot_fadeout"), 2.0f);
			bootImageSequence_->Play(teamNameTitle);
			step_ = TeamLogoUpdate;
			/** FALL THROUGH */
		}
		case TeamLogoUpdate:
		{
			auto* startUpKBC = GetUI<UIIcon>(Hash32("kbcTitle"));
			startUpKBC->color.w = 0.0f;
			if (elapsedTime_ >= MAX_WAIT_TIME) {
				isCompleted_ = true;
			}
			if (elapsedTime_ >= SHORT_WAIT_TIME && g_pad[0]->IsTrigger(enButtonA)){
				isCompleted_ = true;
			}
			break;
		}
	}

	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	bootImageSequence_->Update(deltaTime);
	elapsedTime_ += deltaTime;

	MenuBase::Update();
}


void BootMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void BootMenu::InitializeLogic()
{
	bootImageSequence_ = std::make_unique<UIAnimationSequence>();

	// kbcロゴの表示
	{
		auto* startUpKBC = GetUI<UIIcon>(Hash32("kbcTitle"));
		UIAnimationFactory::Attach<UIColorAnimation>(startUpKBC, Hash32("boot_Fadein"));
		UIAnimationFactory::Attach<UIColorAnimation>(startUpKBC, Hash32("boot_fadeout"));
		bootImageSequence_->Add(Hash32("boot_Fadein")).Add(Hash32("boot_fadeout"), 2.0f);
		bootImageSequence_->Play(startUpKBC);
	}
	// チーム名ロゴの表示
	{

		auto* teamNameTitle = GetUI<UIIcon>(Hash32("teamNameTitle"));
		UIAnimationFactory::Attach<UIColorAnimation>(teamNameTitle, Hash32("boot_Fadein"));
		UIAnimationFactory::Attach<UIColorAnimation>(teamNameTitle, Hash32("boot_fadeout"));
	}
}