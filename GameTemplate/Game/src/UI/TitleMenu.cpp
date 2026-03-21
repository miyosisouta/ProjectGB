/**
 * InGameMenu.cpp
 * タイトルメニュー
 */
#include "stdafx.h"
#include "TitleMenu.h"
#include "UIAnimationFactory.h"

#include "src/Util/TaskSchedulerSystem.h" 


void TitleMenu::Update()
{
	selector_->Update();

	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());
	

	auto* startPositionDummy = GetUI<UIDummy>(Hash32("StartPositionDummy"));
	auto* soundPositionDummy = GetUI<UIDummy>(Hash32("SoundPositionDummy"));
	auto* exitPositionDummy = GetUI<UIDummy>(Hash32("ExitPositionDummy"));
	
	auto* selectIcon = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
	if (selectIcon) {
		const int selectIndex = selector_->GetValue();
		Vector3 selectPositionList[] = {
			startPositionDummy->transform.position,
			soundPositionDummy->transform.position,
			exitPositionDummy->transform.position
		};
		selectIcon->transform.localPosition = selectPositionList[selectIndex];
	}

	if (g_pad[0]->IsTrigger(enButtonA))
	{
		// Aボタンの透明度
		auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
		AbuttonColor->isDraw = false;
		
		// 非表示 はじめる
		auto* start = GetUI<UIIcon>(Hash32("Title_start"));
		start->isDraw = true;

		// 非表示 サウンド
		auto* sound = GetUI<UIIcon>(Hash32("Title_sound"));
		sound->isDraw = true;

		// 非表示 おわる
		auto* exit = GetUI<UIIcon>(Hash32("Title_exit"));
		exit->isDraw = true;

		// 非表示 にくきゅう
		auto* select = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
		select->isDraw = true;

		isAbuttonEnabled = true;
	}

	MenuBase::Update();
}


void TitleMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void TitleMenu::InitializeLogic()
{
	selector_ = std::make_unique<IntSelector>(0, 2, 1, 0);

	if (taskScheduler) {
		delete taskScheduler;
	}
	taskScheduler = new TaskSchedulerSystem();

	// @todo for test
	{
		// がぶっとの文字アニメーション 大きくなる
		auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
		UIAnimationFactory::Attach<UIScaleAnimation>(gabu, Hash32("gabu_scaleUp"));
		taskScheduler->AddTimer(0.0f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				auto* animation = gabu->FindAnimation(Hash32("gabu_scaleUp"));
				animation->Play();
			});
		// がぶっとの文字アニメーション フェード
		UIAnimationFactory::Attach<UIColorAnimation>(gabu, Hash32("gabu_fadeout"));
		taskScheduler->AddTimer(0.0f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				auto* animation = gabu->FindAnimation(Hash32("gabu_fadeout"));
				animation->Play();
			});
		// がぶっとの文字アニメーション 通常サイズに戻る
		UIAnimationFactory::Attach<UIScaleAnimation>(gabu, Hash32("gabu_scaleDown"));
		taskScheduler->AddTimer(0.5f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				auto* animation = gabu->FindAnimation(Hash32("gabu_scaleDown"));
				animation->Play();
			});
		// バスターの文字アニメーション どーん
		auto* bustar = GetUI<UIIcon>(Hash32("Title_bustar"));
		bustar->color.w = 0.0f;
		UIAnimationFactory::Attach<UIColorAnimation>(bustar, Hash32("bustar_fadeout"));
		taskScheduler->AddTimer(1.0f, [&]()
			{
				auto* bustar = GetUI<UIIcon>(Hash32("Title_bustar"));
				auto* animation = bustar->FindAnimation(Hash32("bustar_fadeout"));
				animation->Play();
			});

	}
	{
		// Aボタンの透明度
		auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
		AbuttonColor->isDraw = false;
		AbuttonColor->color.w = 0.0f;
		UIAnimationFactory::Attach<UIColorAnimation>(AbuttonColor, Hash32("pushAbutton_fadeout"));
		taskScheduler->AddTimer(2.0f, [&]()
			{
				auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
				AbuttonColor->isDraw = true;
				AbuttonColor->color.w = 0.3f;
				auto* animation = AbuttonColor->FindAnimation(Hash32("pushAbutton_fadeout"));
				animation->Play();
			});
	}
	{
		// きらきら 右上大
		auto* kiraRightA = GetUI<UIIcon>(Hash32("Title_star_right1"));
		kiraRightA->color.w = 0.0;
		UIAnimationFactory::Attach<UIScaleAnimation>(kiraRightA, Hash32("kira_right_scaleUp"));
		taskScheduler->AddTimer(2.15f, [&]()
			{
				auto* kiraRightA = GetUI<UIIcon>(Hash32("Title_star_right1"));
				kiraRightA->color.w = 1.0f;
				taskScheduler->AddTimer(0.0f, [&]() {
					auto* kiraRightA = GetUI<UIIcon>(Hash32("Title_star_right1"));
					auto* animation = kiraRightA->FindAnimation(Hash32("kira_right_scaleUp"));
					animation->Play();
					},true);
			});
	}
	{
		// きらきら 左上小
		auto* kiraLeftA = GetUI<UIIcon>(Hash32("Title_star_left2"));
		kiraLeftA->color.w = 0.0;
		UIAnimationFactory::Attach<UIScaleAnimation>(kiraLeftA, Hash32("kira_left_scaleUp"));
		taskScheduler->AddTimer(2.0f, [&]()
			{
				auto* kiraLeftA = GetUI<UIIcon>(Hash32("Title_star_left2"));
				kiraLeftA->color.w = 1.0f;
				taskScheduler->AddTimer(0.0f, [&]() {
					auto* kiraLeftA = GetUI<UIIcon>(Hash32("Title_star_left2"));
					auto* animation = kiraLeftA->FindAnimation(Hash32("kira_left_scaleUp"));
					animation->Play();
					}, true);
			});
	}
	{
		// きらきら 右上小
		auto* kiraRightB = GetUI<UIIcon>(Hash32("Title_star_right2"));
		kiraRightB->color.w = 0.0;
		UIAnimationFactory::Attach<UIScaleAnimation>(kiraRightB, Hash32("kira_right_scaleUp"));
		taskScheduler->AddTimer(2.9f, [&]()
			{
				auto* kiraRightB = GetUI<UIIcon>(Hash32("Title_star_right2"));
				kiraRightB->color.w = 1.0f;
				taskScheduler->AddTimer(0.0f, [&]() {
					auto* kiraRightB = GetUI<UIIcon>(Hash32("Title_star_right2"));
					auto* animation = kiraRightB->FindAnimation(Hash32("kira_right_scaleUp"));
					animation->Play();
					}, true);
			});
	}
	{
		// きらきら 左上大
		auto* kiraLeftB = GetUI<UIIcon>(Hash32("Title_star_left1"));
		kiraLeftB->color.w = 0.0;
		UIAnimationFactory::Attach<UIScaleAnimation>(kiraLeftB, Hash32("kira_left_scaleUp"));
		taskScheduler->AddTimer(2.9f, [&]()
			{
				auto* kiraLeftB = GetUI<UIIcon>(Hash32("Title_star_left1"));
				kiraLeftB->color.w = 1.0f;
				taskScheduler->AddTimer(0.0f, [&]() {
					auto* kiraLeftB = GetUI<UIIcon>(Hash32("Title_star_left1"));
					auto* animation = kiraLeftB->FindAnimation(Hash32("kira_left_scaleUp"));
					animation->Play();
					}, true);
			});
	}

	
	
	// 非表示 はじめる
	auto* start = GetUI<UIIcon>(Hash32("Title_start"));
	start->isDraw = false;

	// 非表示 サウンド
	auto* sound = GetUI<UIIcon>(Hash32("Title_sound"));
	sound->isDraw = false;

	// 非表示 おわる
	auto* exit = GetUI<UIIcon>(Hash32("Title_exit"));
	exit->isDraw = false;

	// 非表示 にくきゅう
	auto* select = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
	select->isDraw = false;
}


bool TitleMenu::IsSelectStat() const
{
	// 0は「はじめる」なので
	return selector_->GetValue() == 0;
}


bool TitleMenu::IsSelectSound() const
{
	// 1は「さうんど」なので
	return selector_->GetValue() == 1;
}


bool TitleMenu::IsSelectExit() const
{
	// 2は「おわり」なので
	return selector_->GetValue() == 2;
}