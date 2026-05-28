/**
 * CheckStartWindow.cpp
 * ゲームを始めるかの確認ウィンドウ
 */
#include "stdafx.h"
#include "CheckStartWindow.h"
#include "UIAnimationFactory.h"
#include "Layout.h"

namespace
{
	// 選択
	static int selectButtonType = 0;
	static constexpr int BACK_WINDOW = 0;
	static constexpr int GAME_START = 1;
}


void CheckStartWindow::Update()
{
	// このフレームだけ差分をみたいので一時的に取得
	int oldSelectOptionType = selectButtonType;

	// 選択中
	{
		if (g_pad[0]->IsTrigger(enButtonRight)) {
			selectButtonType++;
			if (selectButtonType > GAME_START) {
				selectButtonType = GAME_START;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
		else if (g_pad[0]->IsTrigger(enButtonLeft)) {
			selectButtonType--;
			if (selectButtonType < BACK_WINDOW) {
				selectButtonType = BACK_WINDOW;
			}
			else {
				SoundManager::Get().PlaySE(enSoundKind_Menu_Move);
			}
		}
	}

	// 選択中のカーソル処理
	{
		if (oldSelectOptionType != selectButtonType)
		{
			// 各ボタンの初期値
			ResetIcon();

			// 通常の枠
			auto* frameBack = GetUI<UIIcon>(Hash32("buttonBackFrame"));
			auto* flameStart = GetUI<UIIcon>(Hash32("buttonStartBackFrame"));
			// 選択されているときの枠
			auto* selectFrame = GetUI<UIDummy>(Hash32("selectColorDummy"));
			// 選択されていないときの枠
			auto* noSelectFrame = GetUI<UIDummy>(Hash32("nonSelectColorDummy"));
			switch (selectButtonType)
			{
				case BACK_WINDOW:
				{
					frameBack->color = selectFrame->color;
					flameStart->color = noSelectFrame->color;
					BackButtonAnim();
					break;
				}
				case GAME_START:
				{
					flameStart->color = selectFrame->color;
					frameBack->color = noSelectFrame->color;
					StartButtonAnim();
					break;
				}
			}
			oldSelectOptionType = selectButtonType;
		}
	}


	MenuBase::Update();
}


void CheckStartWindow::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void CheckStartWindow::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();	
	// 最初のみアニメーション
	BackButtonAnim();
	// 最初のみカラーがつく
	auto* frameBack = GetUI<UIIcon>(Hash32("buttonBackFrame"));
	auto* selectFrame = GetUI<UIDummy>(Hash32("selectColorDummy"));
	frameBack->color = selectFrame->color;
}


void CheckStartWindow::BackButtonAnim()
{
	// もどるボタンの拡大
	auto* buttonBack = GetUI<UIIcon>(Hash32("buttonBack"));
	UIAnimationFactory::Attach<UIScaleAnimation>(buttonBack, Hash32("checkWindowButton_scaleUp"));
	auto* buttonBackAnim = buttonBack->FindAnimation(Hash32("checkWindowButton_scaleUp"));
	buttonBackAnim->Clear();
	buttonBackAnim->Play();

	// もどる枠の拡大
	auto* buttonBackFlame = GetUI<UIIcon>(Hash32("buttonBackFrame"));
	UIAnimationFactory::Attach<UIScaleAnimation>(buttonBackFlame, Hash32("checkWindowFrame_scaleUp"));
	auto* flameBackAnim = buttonBackFlame->FindAnimation(Hash32("checkWindowFrame_scaleUp"));
	flameBackAnim->Clear();
	flameBackAnim->Play();
}

void CheckStartWindow::StartButtonAnim()
{
	// はじめるボタンの拡大
	auto* buttonStart = GetUI<UIIcon>(Hash32("buttonStartBack"));
	UIAnimationFactory::Attach<UIScaleAnimation>(buttonStart, Hash32("checkWindowButton_scaleUp"));
	auto* buttonStartAnim = buttonStart->FindAnimation(Hash32("checkWindowButton_scaleUp"));
	buttonStartAnim->Clear();
	buttonStartAnim->Play();

	// はじめる枠の拡大
	auto* buttonStartFlame = GetUI<UIIcon>(Hash32("buttonStartBackFrame"));
	UIAnimationFactory::Attach<UIScaleAnimation>(buttonStartFlame, Hash32("checkWindowFrame_scaleUp"));
	auto* flameStartAnim = buttonStartFlame->FindAnimation(Hash32("checkWindowFrame_scaleUp"));
	flameStartAnim->Clear();
	flameStartAnim->Play();
}

void CheckStartWindow::ResetIcon()
{
	// 戻るボタンのリセット
	auto* buttonBack = GetUI<UIIcon>(Hash32("buttonBack"));
	auto* buttonBackFlame = GetUI<UIIcon>(Hash32("buttonBackFrame"));

	auto* buttonBackInit = GetUI<UIDummy>(Hash32("scaleDummy"));
	auto* buttonBackFlameInit = GetUI<UIDummy>(Hash32("scaleDummy"));
	// アニメーション停止
	if (auto* buttonBackAnim = GetUI<UIIcon>(Hash32("buttonBack")))
	{
		buttonBackAnim->StopSpriteAnimation();
	}
	if (auto* flameBackAnim = GetUI<UIIcon>(Hash32("buttonBackFrame")))
	{
		flameBackAnim->StopSpriteAnimation();
	}
	// 初期値
	buttonBack->transform.localScale = buttonBackInit->transform.localScale;
	buttonBackFlame->transform.localScale = buttonBackFlameInit->transform.localScale;


	// はじめるボタンのリセット
	auto* buttonStart = GetUI<UIIcon>(Hash32("buttonStartBack"));
	auto* buttonStartFlame = GetUI<UIIcon>(Hash32("buttonStartBackFrame"));

	auto* buttonStartInit = GetUI<UIDummy>(Hash32("scaleDummy"));
	auto* buttonStarFlameInit = GetUI<UIDummy>(Hash32("scaleDummy"));
	// アニメーション停止
	if (auto* buttonStartAnim = GetUI<UIIcon>(Hash32("buttonStartBack")))
	{
		buttonStartAnim->StopSpriteAnimation();
	}
	if (auto* flameStartAnim = GetUI<UIIcon>(Hash32("buttonStartBackFrame")))
	{
		flameStartAnim->StopSpriteAnimation();
	}
	// 初期値
	buttonStart->transform.localScale = buttonStartInit->transform.localScale;
	buttonStartFlame->transform.localScale = buttonStarFlameInit->transform.localScale;
}

bool CheckStartWindow::IsSelectBackButton() const
{
	return selectButtonType == BACK_WINDOW;
}

bool CheckStartWindow::IsSelectStartButton() const
{
	return selectButtonType == GAME_START;
}
