/**
 * TimerMenu.cpp
 * ゲームクリア時の演出等用
 */
#include "stdafx.h"
#include "TimerMenu.h"
#include "UIAnimationFactory.h"


namespace
{
	const float ONE_LAP = 360.0;	// 一回転360
	const int DIGIT_BASE = 10;		// 桁を割るためにいる
}


void TimerMenu::Update()
{
	// 針UIの作成
	auto* hari = GetUI<UIIcon>(Hash32("clock_hari"));
	// 針UIの作成
	auto* flashingEffect = GetUI<UIIcon>(Hash32("clock_effect"));

	// 回転
	float angle = progressRate_ * ONE_LAP;			// 回転角度
	hari->transform.localRotation.SetRotationDegZ(angle);		// z座標を軸にQuaternionを制作
	flashingEffect->transform.localRotation.SetRotationDegZ(angle);

	// 分単位のUI作成
	// セミコロン
	auto* colon = GetUI<UIIcon>(Hash32("clock_colon"));
	// 分単位の２桁目
	auto* minUI2 = GetUI<UIDigit>(Hash32("clockMin2"));
	// 分単位の１桁目
	auto* minUI1 = GetUI<UIDigit>(Hash32("clockMin1"));
	// 分単位の計算
	int minTens = minutes_ / DIGIT_BASE;
	minUI2->SetNumber(minTens);
	int minOnes = minutes_ % DIGIT_BASE;
	minUI1->SetNumber(minOnes);

	// 秒単位のUI作成
	// 秒単位の２桁目
	auto* secUI2 = GetUI<UIDigit>(Hash32("clockSec2"));
	// 秒単位の１桁目
	auto* secUI1 = GetUI<UIDigit>(Hash32("clockSec1"));
	// 秒単位の計算
	int secTens = seconds_ / DIGIT_BASE;
	secUI2->SetNumber(secTens);
	int secOnes = seconds_ % DIGIT_BASE;
	secUI1->SetNumber(secOnes);


	if (flashingStart_)
	{
		// タイムの点滅処理
		UIAnimationFactory::Attach<UIColorAnimation>(colon, Hash32("timer_color"));
		auto* colorAnimation = colon->FindAnimation(Hash32("timer_color"));
		colorAnimation->Play();
		//	分単位
		UIAnimationFactory::Attach<UIColorAnimation>(minUI2, Hash32("timer_color"));
		colorAnimation = minUI2->FindAnimation(Hash32("timer_color"));
		colorAnimation->Play();
		UIAnimationFactory::Attach<UIColorAnimation>(minUI1, Hash32("timer_color"));
		colorAnimation = minUI1->FindAnimation(Hash32("timer_color"));
		colorAnimation->Play();
		// 秒単位
		UIAnimationFactory::Attach<UIColorAnimation>(secUI2, Hash32("timer_color"));
		colorAnimation = secUI2->FindAnimation(Hash32("timer_color"));
		colorAnimation->Play();
		UIAnimationFactory::Attach<UIColorAnimation>(secUI1, Hash32("timer_color"));
		colorAnimation = secUI1->FindAnimation(Hash32("timer_color"));
		colorAnimation->Play();
		// 針のエフェクト
		//auto* flashingEffect = GetUI<UIIcon>(Hash32("clock_effect"));
		flashingEffect->isDraw = true;
		UIAnimationFactory::Attach<UIColorAnimation>(flashingEffect, Hash32("effect_color"));
		colorAnimation = flashingEffect->FindAnimation(Hash32("effect_color"));
		colorAnimation->Play();
	}


	MenuBase::Update();
}


void TimerMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void TimerMenu::InitializeLogic()
{
	// ゲームクリアの文字をバウンスする
	/*auto* gameClearBounce = GetUI<UIIcon>(Hash32("gameClear"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceA"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceB"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(gameClearBounce, Hash32("GameClearStampBounceC"));
	gameClearSequence_ = std::make_unique<UIAnimationSequence>();
	gameClearSequence_->Add(Hash32("GameClearStampBounceA")).Add(Hash32("GameClearStampBounceB")).Add(Hash32("GameClearStampBounceC"));*/

}