/**
 * PlayGameMenu.cpp
 * ボス選択メニュー
 */
#include "stdafx.h"
#include "PlayGameMenu.h"
#include "UIAnimationFactory.h"
#include "Layout.h"


namespace
{
	// ボス
	static int pageNumber = 0;
	static constexpr int PAGE_A = 0;
	static constexpr int PAGE_B = 1;
	static constexpr int PAGE_C = 2;
}


void PlayGameMenu::Update()
{
	taskSchedulerSystem_->Update(g_gameTime->GetFrameDeltaTime());

	// このフレームだけ差分をみたいので一時的に取得
	int oldSelectOptionType = pageNumber;

	// 選択
	{
		if (g_pad[0]->IsTrigger(enButtonRight)) {
			pageNumber++;
			if (pageNumber > PAGE_C) {
				pageNumber = PAGE_C;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonLeft)) {
			pageNumber--;
			if (pageNumber < PAGE_A) {
				pageNumber = PAGE_A;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}
	// それぞれのページの表示
	{
		// ページ
		auto* page1 = GetUI<UIIcon>(Hash32("no1"));
		auto* page2 = GetUI<UIIcon>(Hash32("no2"));
		auto* page3 = GetUI<UIIcon>(Hash32("no3"));

		// 矢印
		auto* leftArrow = GetUI<UIIcon>(Hash32("leftArrow"));
		auto* rightArrow = GetUI<UIIcon>(Hash32("rightArrow"));
		ArrowAnimation();

		switch (pageNumber)
		{
			case PAGE_A:
			{
				page1->isDraw = true;
				page2->isDraw = false;
				page3->isDraw = false;
				
				leftArrow->isDraw = false;
				rightArrow->isDraw = true;

				break;
			}
			 case PAGE_B:
			{
				 page1->isDraw = false;
				 page2->isDraw = true;
				 page3->isDraw = false;

				 leftArrow->isDraw = true;
				 rightArrow->isDraw = true;

				break;
			}
			case PAGE_C:
			{
				page1->isDraw = false;
				page2->isDraw = false;
				page3->isDraw = true;

				leftArrow->isDraw = true;
				rightArrow->isDraw = false;

				break;
			}
		}
	}
		
	MenuBase::Update();
}


void PlayGameMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void PlayGameMenu::InitializeLogic()
{
	// 選択状態をリセット
	pageNumber = PAGE_A;

	// taskSchedulerを先に生成
	taskSchedulerSystem_ = std::make_unique<TaskSchedulerSystem>();

	const int id = taskSchedulerSystem_->CreateLoopSequence(20.0f);
	{
		nikukyuList_[0] = GetUI<UIIcon>(Hash32("optionBack/nikukyu1"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[0], Hash32("nikukyuu_fadein"));
		taskSchedulerSystem_->AddLoopTimer(id, 0.0f, [&]()
			{
				auto* animation = nikukyuList_[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskSchedulerSystem_->AddLoopTimer(id, 6.0f, [&]()
			{
				auto* animation = nikukyuList_[0]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList_[1] = GetUI<UIIcon>(Hash32("optionBack/nikukyu2"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[1], Hash32("nikukyuu_fadein"));
		taskSchedulerSystem_->AddLoopTimer(id, 4.0f, [&]()
			{
				auto* animation = nikukyuList_[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskSchedulerSystem_->AddLoopTimer(id, 10.0f, [&]()
			{
				auto* animation = nikukyuList_[1]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList_[2] = GetUI<UIIcon>(Hash32("optionBack/nikukyu3"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[2], Hash32("nikukyuu_fadein"));
		taskSchedulerSystem_->AddLoopTimer(id, 8.0f, [&]()
			{
				auto* animation = nikukyuList_[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskSchedulerSystem_->AddLoopTimer(id, 14.0f, [&]()
			{
				auto* animation = nikukyuList_[2]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
	{
		nikukyuList_[3] = GetUI<UIIcon>(Hash32("optionBack/nikukyu4"));
		UIAnimationFactory::Attach<UIColorAnimation>(nikukyuList_[3], Hash32("nikukyuu_fadein"));
		taskSchedulerSystem_->AddLoopTimer(id, 12.0f, [&]()
			{
				auto* animation = nikukyuList_[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Play();
			});
		taskSchedulerSystem_->AddLoopTimer(id, 18.0f, [&]()
			{
				auto* animation = nikukyuList_[3]->FindAnimation(Hash32("nikukyuu_fadein"));
				animation->Stop();
			}, true);
	}
}

void PlayGameMenu::ArrowAnimation()
{
	// 左に動く
	auto* leftArrow = GetUI<UIIcon>(Hash32("leftArrow"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(leftArrow, Hash32("arrow_posLeft"));
	auto* leftAnimeion = leftArrow->FindAnimation(Hash32("arrow_posLeft"));
	leftAnimeion->Play();

	// 右に動く
	auto* rightArrow = GetUI<UIIcon>(Hash32("rightArrow"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(rightArrow, Hash32("arrow_posRight"));
	auto* rightAnimeion = rightArrow->FindAnimation(Hash32("arrow_posRight"));
	rightAnimeion->Play();
}


