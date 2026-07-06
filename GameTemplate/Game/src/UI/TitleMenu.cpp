/**
 * InGameMenu.cpp
 * タイトルメニュー
 */
#include "stdafx.h"
#include "TitleMenu.h"
#include "UIAnimationFactory.h"


void TitleMenu::Update()
{
	if (isAbuttonEnabled)
	{
		selector_->Update();
		if (selector_->IsChanged()) {
			SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
		}
	}

	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());
	

	auto* startPositionDummy = GetUI<UIDummy>(Hash32("StartPositionDummy"));
	auto* OptionPositionDummy = GetUI<UIDummy>(Hash32("OptionPositionDummy"));
	auto* exitPositionDummy = GetUI<UIDummy>(Hash32("ExitPositionDummy"));
	
	auto* selectIcon = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
	if (selectIcon) {
		const int selectIndex = selector_->GetValue();
		Vector3 selectPositionList[] = {
			startPositionDummy->transform.position,
			OptionPositionDummy->transform.position,
			exitPositionDummy->transform.position
		};
		selectIcon->transform.localPosition = selectPositionList[selectIndex];
	}

	if (g_pad[0]->IsTrigger(enButtonA))
	{
		bool canDo = false;

		// Aボタンの透明度
		auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
		if (AbuttonColor->color.w > 0.0f) {
			canDo = true;
		}
		if(canDo)
		{
			SoundManager::Get().PlaySE(enSoundKind_Menu_Decide);

			AbuttonColor->isDraw = false;

			// 非表示 はじめる
			auto* start = GetUI<UIIcon>(Hash32("Title_start"));
			start->isDraw = true;

			// 非表示 サウンド
			auto* sound = GetUI<UIIcon>(Hash32("Title_option"));
			sound->isDraw = true;

			// 非表示 おわる
			auto* exit = GetUI<UIIcon>(Hash32("Title_exit"));
			exit->isDraw = true;

			// 非表示 にくきゅう
			auto* select = GetUI<UIIcon>(Hash32("Title_nikukyu_button"));
			select->isDraw = true;

			isAbuttonEnabled = true;
		}
	}

	effectRender->Update(g_gameTime->GetFrameDeltaTime());
	effectRenderB->Update(g_gameTime->GetFrameDeltaTime());

	MenuBase::Update();
}


void TitleMenu::Render(RenderContext& rc)
{	
	effectRender->Draw(rc);
	effectRenderB->Draw(rc);

	MenuBase::Render(rc);
}


void TitleMenu::InitializeLogic()
{
	selector_ = std::make_unique<IntSelector>(0, 2, 1, 0);

	taskScheduler = std::make_unique<TaskSchedulerSystem>();

	{
		// がぶっとの文字アニメーション 大きくなる
		auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
		gabu->isDraw = false;
		UIAnimationFactory::Attach<UIScaleAnimation>(gabu, Hash32("gabu_scaleUp"));
		taskScheduler->AddTimer(2.0f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				gabu->isDraw = true;
				auto* animation = gabu->FindAnimation(Hash32("gabu_scaleUp"));
				animation->Play();
			});
		// がぶっとの文字アニメーション フェード
		UIAnimationFactory::Attach<UIColorAnimation>(gabu, Hash32("gabu_fadeout"));
		taskScheduler->AddTimer(2.0f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				auto* animation = gabu->FindAnimation(Hash32("gabu_fadeout"));
				animation->Play();
			});
		// がぶっとの文字アニメーション 通常サイズに戻る
		UIAnimationFactory::Attach<UIScaleAnimation>(gabu, Hash32("gabu_scaleDown"));
		taskScheduler->AddTimer(2.5f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				auto* animation = gabu->FindAnimation(Hash32("gabu_scaleDown"));
				animation->Play();
			});
		// がぶっとの文字アニメーション 通常サイズに戻る
		UIAnimationFactory::Attach<UIScaleAnimation>(gabu, Hash32("gabu_normalscale"));
		taskScheduler->AddTimer(2.8f, [&]()
			{
				auto* gabu = GetUI<UIIcon>(Hash32("Title_gabu"));
				auto* animation = gabu->FindAnimation(Hash32("gabu_normalscale"));
				animation->Play();
			});
		// バスターの文字アニメーション どーん
		auto* bustar = GetUI<UIIcon>(Hash32("Title_bustar"));
		bustar->color.w = 0.0f;
		UIAnimationFactory::Attach<UIColorAnimation>(bustar, Hash32("bustar_fadeout"));
		taskScheduler->AddTimer(3.0f, [&]()
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
		taskScheduler->AddTimer(4.0f, [&]()
			{
				auto* AbuttonColor = GetUI<UIIcon>(Hash32("Title_push_Abutton"));
				AbuttonColor->isDraw = true;
				AbuttonColor->color.w = 0.3f;
				auto* animation = AbuttonColor->FindAnimation(Hash32("pushAbutton_fadeout"));
				animation->Play();
			});
	}
	{
		// 星のエフェクト
		effectRender = std::make_unique<ParticleEffectRender>();
		effectRender->Init("Assets/ui/vfx/effect_sparkle.json", "Assets/ui/titleUI/kira.dds", 128.0f, 128.0f);
		effectRender->SetPosition(Vector3(280.0f, 290, 0.0f));
		effectRender->EnableHotReload();

		effectRenderB = std::make_unique<ParticleEffectRender>();
		effectRenderB->Init("Assets/ui/vfx/effect_sparkle.json", "Assets/ui/titleUI/kira.dds", 128.0f, 128.0f);
		effectRenderB->SetPosition(Vector3(-250.0f, 200, 0.0f));
		effectRenderB->EnableHotReload();

		taskScheduler->AddTimer(4.0f, [&]()
			{
				effectRender->Play();
				effectRenderB->Play();
			});
	}
	

	// 非表示 はじめる
	auto* start = GetUI<UIIcon>(Hash32("Title_start"));
	start->isDraw = false;

	// 非表示 設定
	auto* option = GetUI<UIIcon>(Hash32("Title_option"));
	option->isDraw = false;

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


bool TitleMenu::IsSelectOption() const
{
	// 1は「設定」なので
	return selector_->GetValue() == 1;
}


bool TitleMenu::IsSelectExit() const
{
	// 2は「おわり」なので
	return selector_->GetValue() == 2;
}

void TitleMenu::StopEffect()
{
	effectRender->Reset();
	effectRenderB->Reset();
}
