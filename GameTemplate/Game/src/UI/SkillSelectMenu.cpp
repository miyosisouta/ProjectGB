/**
 * SkillSelectMenu.cpp
 * ボス選択メニュー
 */
#include "stdafx.h"
#include "SkillSelectMenu.h"
#include "UIAnimationFactory.h"
#include "Layout.h"


namespace
{
	// スキルタイプ
	static int selectSkillType = 0;
	static constexpr int SKILL_LANDMINE_TYPE = 0;
	static constexpr int SKILL_BITE_TYPE = 1;
	static constexpr int SKILL_FIRE_TYPE = 2;
}


void SkillSelectMenu::Update()
{
	taskSchedulerSystem_->Update(g_gameTime->GetFrameDeltaTime());


	// このフレームだけ差分をみたいので一時的に取得
	int oldSelectOptionType = selectSkillType;

	// 選択
	{
		if (g_pad[0]->IsTrigger(enButtonRight)) {
			selectSkillType++;
			if (selectSkillType > SKILL_FIRE_TYPE) {
				selectSkillType = SKILL_FIRE_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonLeft)) {
			selectSkillType--;
			if (selectSkillType < SKILL_LANDMINE_TYPE) {
				selectSkillType = SKILL_LANDMINE_TYPE;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}
	// 選択中のカーソル処理
	{
		// 青色の枠
		auto* selectFrameLandmine = GetUI<UIIcon>(Hash32("landmine_flame"));
		auto* selectFlameBite = GetUI<UIIcon>(Hash32("bite_flame"));
		auto* selectFlameFire = GetUI<UIIcon>(Hash32("fire_flame"));

		switch (selectSkillType)
		{
			case SKILL_LANDMINE_TYPE:
			{
				selectFrameLandmine->isDraw = true;
				selectFlameBite->isDraw = false;
				selectFlameFire->isDraw = false;
				break;
			}
			case SKILL_BITE_TYPE:
			{
				selectFrameLandmine->isDraw = false;
				selectFlameBite->isDraw = true;
				selectFlameFire->isDraw = false;
				break;
			}
			case SKILL_FIRE_TYPE:
			{
				selectFrameLandmine->isDraw = false;
				selectFlameBite->isDraw = false;
				selectFlameFire->isDraw = true;
				break;
			}
		}
	}


	// スキル
	{
		if (g_pad[0]->IsTrigger(enButtonA))
		{
			AbilityType skillType = AbilityType::enDefault;
			switch (selectSkillType)
				{
				case SKILL_LANDMINE_TYPE:
				{
					skillType = AbilityType::enLandmine;
					break;
				}

				case SKILL_BITE_TYPE:
				{
					skillType = AbilityType::enDefault;
					break;
				}

				case SKILL_FIRE_TYPE:
				{
					skillType = AbilityType::enFireMagic;
					break;
				}
			}
		}
		CharacterDataBase::Get().GetPlayerParam().ability;
	}

	MenuBase::Update();
}


void SkillSelectMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void SkillSelectMenu::InitializeLogic()
{
	// 選択状態を一番左(地雷)にリセット
	selectSkillType = SKILL_LANDMINE_TYPE;

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

bool SkillSelectMenu::IsSelectSkillLandMine() const
{
	return selectSkillType == SKILL_LANDMINE_TYPE;
}

bool SkillSelectMenu::IsSelectSkillBite() const
{
	return selectSkillType == SKILL_BITE_TYPE;
}

bool SkillSelectMenu::IsSelectSkillFire() const
{
	return selectSkillType == SKILL_FIRE_TYPE;
}

