/**
 * BossSelectMenu.cpp
 * ボス選択メニュー
 */
#include "stdafx.h"
#include "BossSelectMenu.h"
#include "UIAnimationFactory.h"
#include "Layout.h"


namespace
{
	// ボス
	static int selectBossType = 0;
	static constexpr int BOSS_GOLLIRA_TYPE = 0;
	static constexpr int BOSS_TURTLE_TYPE = 1;
}


void BossSelectMenu::Update()
{
	taskSchedulerSystem_->Update(g_gameTime->GetFrameDeltaTime());

	// このフレームだけ差分をみたいので一時的に取得
	int oldSelectOptionType = selectBossType;

	// 選択
	{
		if (g_pad[0]->IsTrigger(enButtonRight)) {
			selectBossType++;
			if (selectBossType > BOSS_TURTLE_TYPE) {
				selectBossType = BOSS_TURTLE_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonLeft)) {
			selectBossType--;
			if (selectBossType < BOSS_GOLLIRA_TYPE) {
				selectBossType = BOSS_GOLLIRA_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}
	// 選択中のカーソル処理
	{
		// 青色の枠
		auto* selectFrameGollira = GetUI<UIIcon>(Hash32("bossSelect_Frame_Gollira"));
		auto* selectFlameTurtle = GetUI<UIIcon>(Hash32("bossSelect_Frame_Turtle"));
		switch (selectBossType)
		{
			case BOSS_GOLLIRA_TYPE:
			{
				selectFrameGollira->isDraw = true;
				selectFlameTurtle->isDraw = false;
				
				break;
			}
			case BOSS_TURTLE_TYPE:
			{
				selectFlameTurtle->isDraw = true;
				selectFrameGollira->isDraw = false;
				break;
			}
		}
	}

	// 選択中のアイコン
	{
		if (oldSelectOptionType != selectBossType)
		{
			// タスクスケジューラーを消す
			taskSchedulerSystem_->ClearSchedule();

			// 各アイコンの初期値
			AnimationResetIcon();

			switch (selectBossType)
			{
				case BOSS_GOLLIRA_TYPE:
				{
					GolliraUpdateAnimation();
					break;
				}
				case BOSS_TURTLE_TYPE:
				{
					TurtleUpdateAnimation();
					break;
				}
			}
			oldSelectOptionType = selectBossType;
		}
	}

	
	MenuBase::Update();
}


void BossSelectMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void BossSelectMenu::InitializeLogic()
{
	// 選択状態をゴリラ（左端）にリセット
	selectBossType = BOSS_GOLLIRA_TYPE;

	// taskSchedulerを先に生成
	taskSchedulerSystem_ = std::make_unique<TaskSchedulerSystem>();

	// アニメーションを先にアタッチ
	AnimationIconAttach();

	// アイコンリセット→初期アニメーション再生
	AnimationResetIcon();
	GolliraUpdateAnimation();

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

	// 一度だけ再生
	GolliraUpdateAnimation();
}

void BossSelectMenu::AnimationIconAttach()
{
	// ゴリラアイコンのアニメーションアタッチ
	auto* gollira = GetUI<UIIcon>(Hash32("boss_gorilla"));
	UIAnimationFactory::Attach<UIScaleAnimation>(gollira, Hash32("boss_gorillaIcon_scaleUp"));
	UIAnimationFactory::Attach<UIScaleAnimation>(gollira, Hash32("boss_gorillaIcon_scaleDown"));
	UIAnimationFactory::Attach<UIScaleAnimation>(gollira, Hash32("boss_gorillaIcon_scaleNormal"));

	// カメアイコンのアニメーションアタッチ
	auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle, Hash32("boss_turtleIcon_posUp"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(turtle, Hash32("boss_turtleIcon_posNormal"));
	UIAnimationFactory::Attach<UIScaleAnimation>(turtle, Hash32("boss_turtleIcon_scaleDown"));
	UIAnimationFactory::Attach<UIScaleAnimation>(turtle, Hash32("boss_turtleIcon_scaleNormal"));
}

void BossSelectMenu::AnimationResetIcon()
{
	// ゴリラアイコン
	auto* gollira = GetUI<UIIcon>(Hash32("boss_gorilla"));
	auto* golliraInit = GetUI<UIDummy>(Hash32("InitGollira"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("boss_gorilla")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	gollira->transform.localScale = golliraInit->transform.localScale;

	// カメアイコン
	auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
	auto* turtleInit = GetUI<UIDummy>(Hash32("InitTurtle"));
	// アニメーション停止
	if (auto* animation = GetUI<UIIcon>(Hash32("boss_turtle")))
	{
		animation->StopSpriteAnimation();
	}
	// 初期値
	turtle->transform.localScale = turtleInit->transform.localScale;
	turtle->transform.localPosition = turtleInit->transform.localPosition;
}

void BossSelectMenu::GolliraUpdateAnimation()
{
	// タスクスケジューラー
	const int id = taskSchedulerSystem_->CreateLoopSequence(3.0f);
	taskSchedulerSystem_->AddLoopTimer(id, 1.0f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_gorilla"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_gorillaIcon_scaleUp"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 1.3f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_gorilla"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_gorillaIcon_scaleUp"));
			animation->Stop();
		}, false);
	taskSchedulerSystem_->AddLoopTimer(id, 1.4f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_gorilla"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_gorillaIcon_scaleDown"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 1.7f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_gorilla"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_gorillaIcon_scaleDown"));
			animation->Stop();
		}, false);
	taskSchedulerSystem_->AddLoopTimer(id, 1.8f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_gorilla"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_gorillaIcon_scaleNormal"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 2.1f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_gorilla"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_gorillaIcon_scaleNormal"));
			animation->Stop();
		}, false);
}

void BossSelectMenu::TurtleUpdateAnimation()
{
	// タスクスケジューラー
	const int id = taskSchedulerSystem_->CreateLoopSequence(3.0f);
	taskSchedulerSystem_->AddLoopTimer(id, 1.0f, [&]()
		{
			auto* gorilla = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = gorilla->FindAnimation(Hash32("boss_turtleIcon_scaleDown"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 1.2f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleDown"));
			animation->Stop();
		}, false);

	taskSchedulerSystem_->AddLoopTimer(id, 1.2f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleNormal"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 1.5f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleNormal"));
			animation->Stop();
		}, false);

	taskSchedulerSystem_->AddLoopTimer(id, 1.2f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_posUp"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 2.0f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_posUp"));
			animation->Stop();
		}, false);

	
	taskSchedulerSystem_->AddLoopTimer(id, 2.0f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_posNormal"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 2.3f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_posNormal"));
			animation->Stop();
		}, false);
	taskSchedulerSystem_->AddLoopTimer(id, 2.2f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleDown"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 2.4f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleDown"));
			animation->Stop();
		}, false);
	taskSchedulerSystem_->AddLoopTimer(id, 2.4f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleNormal"));
			animation->Clear();
			animation->Play();
		});
	taskSchedulerSystem_->AddLoopTimer(id, 2.7f, [&]()
		{
			auto* turtle = GetUI<UIIcon>(Hash32("boss_turtle"));
			auto* animation = turtle->FindAnimation(Hash32("boss_turtleIcon_scaleNormal"));
			animation->Stop();
		}, false);
}

bool BossSelectMenu::IsSelectGollira() const
{
	return selectBossType == BOSS_GOLLIRA_TYPE;
}

bool BossSelectMenu::IsSelectTurtle() const
{
	return selectBossType == BOSS_TURTLE_TYPE;
}
