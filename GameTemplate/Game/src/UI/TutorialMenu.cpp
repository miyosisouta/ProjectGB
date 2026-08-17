/**
 * TutorialMenu.cpp
 * チュートリアル
 */
#include "stdafx.h"
#include "TutorialMenu.h"
#include "UIAnimationFactory.h"
#include "Layout.h"
#include "src/Core/ParameterManager.h"

namespace
{
	/** MoveAnimation(back/textの移動演出)の再生時間。演出クリップの長さに合わせて調整する */
	constexpr float MOVE_ANIMATION_DURATION = 2.0f;
}


uint32_t TutorialMenu::GetIconHash(TutorialManager::Stage stage)
{
	using Stage = TutorialManager::Stage;
	switch (stage)
	{
	case Stage::Move:         return Hash32("idou");
	case Stage::NormalAttack: return Hash32("attack");
	case Stage::Skill:        return Hash32("skill");
	case Stage::Avoid:        return Hash32("avoid");
	case Stage::JustAvoid:    return Hash32("justAvoid");
	case Stage::Free:         return Hash32("freeMove");
	default:                  return 0;
	}
}


void TutorialMenu::FadeInText(TutorialManager::Stage stage)
{
	auto* icon = GetUI<UIIcon>(GetIconHash(stage));
	if (!icon) { return; }

	// jsonの初期値はisDraw:falseなので、フェードインの前に明示的に描画対象へする
	icon->isDraw = true;

	UIAnimationFactory::Attach<UIColorAnimation>(icon, Hash32("tutorialText_fadeIn"));
	auto* fadeInAnimation = icon->FindAnimation(Hash32("tutorialText_fadeIn"));
	fadeInAnimation->Play();

	currentIcon_ = icon;
}


void TutorialMenu::FadeOutText()
{
	if (!currentIcon_) { return; }

	UIAnimationFactory::Attach<UIColorAnimation>(currentIcon_, Hash32("tutorialText_fadeOut"));
	auto* fadeOutAnimation = currentIcon_->FindAnimation(Hash32("tutorialText_fadeOut"));
	fadeOutAnimation->Play();

	// isDrawはここでは触らない。アニメーションがアルファを0まで下げることで見た目上消える。
	// Play()直後にisDraw=falseへ落とすと、フェードのアルファ変化を待たずに即座に描画自体が止まってしまう

	currentIcon_ = nullptr;
}

void TutorialMenu::FrameAnimation()
{
	taskScheduler->AddTimer(0.0f, [&]()
		{
			auto* flamePika = GetUI<UIIcon>(Hash32("pika"));
			flamePika->isDraw = true;
			UIAnimationFactory::Attach<UIColorAnimation>(flamePika, Hash32("tutorialFrame_fadeOut"));
			auto* flameAnim = flamePika->FindAnimation(Hash32("tutorialFrame_fadeOut"));
			flameAnim->Play();
		});
	taskScheduler->AddTimer(1.5, [&]()
		{
			// 青枠点滅：消
			auto* flamePika = GetUI<UIIcon>(Hash32("pika"));
			auto* dummyFlame = GetUI<UIDummy>(Hash32("InitFlame"));
			auto* animation = flamePika->FindAnimation(Hash32("tutorialFrame_fadeOut"));
			animation->Clear();
			flamePika->color.w = dummyFlame->color.w;
			flamePika->isDraw = false;
		});
}


void TutorialMenu::MoveAnimation()
{
	taskScheduler->AddTimer(1.0f, [&]()
		{
			// 背景が右に動く
			auto* rightMove = GetUI<UIIcon>(Hash32("back"));
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(rightMove, Hash32("tutorial_posRight"));
			auto* rightAnimeion = rightMove->FindAnimation(Hash32("tutorial_posRight"));
			rightAnimeion->Play();

			// 右に動く
			auto* rightMove2 = GetUI<UIIcon>(Hash32("text"));
			UIAnimationFactory::Attach<UITranslateOffsetAnimation>(rightMove2, Hash32("tutorialText_posRight"));
			auto* rightAnimeion2 = rightMove2->FindAnimation(Hash32("tutorialText_posRight"));
			rightAnimeion2->Play();
		});
}


void TutorialMenu::Update()
{
	// TutorialManagerが「アニメーション」表示中に切り替わった瞬間（false→trueのエッジ）を検知して、
	// 今表示中のアイコン（クリアした段階のもの）をフェードアウトさせる。
	// GetUIAnimationFlag()（1フレームだけのパルス）は、Avoid/JustAvoidの遷移がCollisionHitManagerの
	// コールバック経由（BattleManager::Update()内でuiManager_.Update()より後に実行される）で発火するため、
	// Menu側のUpdate()がその1フレームを取りこぼすことがある。IsTransitioning()は継続値なので取りこぼさない
	const bool isTransitioning = TutorialManager::Get().IsTransitioning();
	if (isTransitioning && !wasTransitioning_)
	{
		switch (TutorialManager::Get().GetStage())
		{
		case TutorialManager::Stage::Move:
			FrameAnimation();
			FadeOutText(); // 「いどう」クリア → idou をフェードアウト
			break;
		case TutorialManager::Stage::NormalAttack:
			FrameAnimation();
			FadeOutText(); // 「つうじょう」クリア → attack をフェードアウト
			break;
		case TutorialManager::Stage::Skill:
			FrameAnimation();
			FadeOutText(); // 「すきる」クリア → skill をフェードアウト
			break;
		case TutorialManager::Stage::Avoid:
			FrameAnimation();
			FadeOutText(); // 「かいひ」クリア → avoid をフェードアウト
			break;
		case TutorialManager::Stage::JustAvoid:
			FrameAnimation();
			FadeOutText(); // 「ジャスト」クリア → justAvoid をフェードアウト
			break;
		default:
			break;
		}
	}
	wasTransitioning_ = isTransitioning;

	// TutorialManager側の段階(stage_)が実際に切り替わった瞬間を、直接の値変化として毎フレーム検知する
	const TutorialManager::Stage currentStage = TutorialManager::Get().GetStage();
	if (!TutorialManager::Get().IsFinished() && currentStage != lastFadedStage_)
	{
		switch (currentStage)
		{
		case TutorialManager::Stage::NormalAttack: FadeInText(TutorialManager::Stage::NormalAttack); break;
		case TutorialManager::Stage::Skill:        FadeInText(TutorialManager::Stage::Skill);        break;
		case TutorialManager::Stage::Avoid:        FadeInText(TutorialManager::Stage::Avoid);        break;
		case TutorialManager::Stage::JustAvoid:    FadeInText(TutorialManager::Stage::JustAvoid);    break;
		case TutorialManager::Stage::Free:         FadeInText(TutorialManager::Stage::Free);         break;
		default: break;
		}
		lastFadedStage_ = currentStage;
	}

	// チュートリアル終了（Free到達 or Enterでスキップ）時、後始末を一度だけ行う
	if (!finishedHandled_ && TutorialManager::Get().IsFinished())
	{
		finishedHandled_ = true;
		FadeOutText();
	}

	taskScheduler->Update(g_gameTime->GetFrameDeltaTime());
	startIconTaskScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	MenuBase::Update();
}


void TutorialMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void TutorialMenu::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();
	startIconTaskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// Move段階が始まる前の導入演出（back/textの移動）を再生する
	MoveAnimation();

	// 導入演出が終わるタイミングで、「いどう」アイコン(idou)をフェードインする
	taskScheduler->AddTimer(MOVE_ANIMATION_DURATION, [&]()
		{
			FadeInText(TutorialManager::Stage::Move);
			lastFadedStage_ = TutorialManager::Stage::Move; // 初回分をここで確定させておく
		});
}