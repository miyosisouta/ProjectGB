/**
 * UIAnimation.cpp
 * UI用のアニメーション処理群
 */
#include "stdafx.h"
#include "UIAnimation.h"
#include "UIParts.h"


UIColorAnimation::UIColorAnimation()
{
	// applyFunc_ は Update() 内で直接 ui_->color に書き込むため不要
}

void UIColorAnimation::Update()
{
	bool wasPlaying = curve_.IsPlaying();
	curve_.Update(g_gameTime->GetFrameDeltaTime());
	if ((curve_.IsPlaying() || wasPlaying) && ui_)
	{
		ui_->color = curve_.GetCurrentValue();
	}
}


/*******************************************************/


UIScaleAnimation::UIScaleAnimation()
{
}

void UIScaleAnimation::Update()
{
	bool wasPlaying = curve_.IsPlaying();
	curve_.Update(g_gameTime->GetFrameDeltaTime());
	if ((curve_.IsPlaying() || wasPlaying) && ui_)
	{
		ui_->transform.localScale = curve_.GetCurrentValue();
	}
}


/*******************************************************/


UITranslateAnimation::UITranslateAnimation()
{
}

void UITranslateAnimation::Update()
{
	bool wasPlaying = curve_.IsPlaying();
	curve_.Update(g_gameTime->GetFrameDeltaTime());
	if ((curve_.IsPlaying() || wasPlaying) && ui_)
	{
		ui_->transform.localPosition = curve_.GetCurrentValue();
	}
}


/*******************************************************/


UITranslateOffsetAnimation::UITranslateOffsetAnimation()
{
}

void UITranslateOffsetAnimation::Update()
{
	bool wasPlaying = curve_.IsPlaying();
	curve_.Update(g_gameTime->GetFrameDeltaTime());
	if ((curve_.IsPlaying() || wasPlaying) && ui_)
	{
		ui_->transform.localPosition.Add(curve_.GetCurrentValue());
	}
}


/*******************************************************/


UIRotationAnimation::UIRotationAnimation()
{
}

void UIRotationAnimation::Update()
{
	bool wasPlaying = curve_.IsPlaying();
	curve_.Update(g_gameTime->GetFrameDeltaTime());
	if ((curve_.IsPlaying() || wasPlaying) && ui_)
	{
		ui_->transform.localRotation.SetRotationDegZ(curve_.GetCurrentValue());
	}
}




/*******************************************************/


void UICanvasFadeAnimation::Update()
{
	bool wasPlaying = curve_.IsPlaying();
	curve_.Update(g_gameTime->GetFrameDeltaTime());
	if ((curve_.IsPlaying() || wasPlaying) && ui_)
	{
		ui_->color.w = curve_.GetCurrentValue();
	}
}


/*******************************************************/


void UIAnimationSequence::Update(float deltaTime)
{
	if (!isPlaying_ || !target_) return;

	// ディレイ待ち
	if (waitingDelay_) {
		delayTimer_ -= deltaTime;
		if (delayTimer_ > 0.0f) return;
		waitingDelay_ = false;
		StartCurrentStep();
		return;
	}

	// 現在のアニメーション完了チェック
	if (currentIndex_ >= 0 && currentIndex_ < static_cast<int>(steps_.size())) {
		const auto& step = steps_[currentIndex_];
		UIAnimationBase* anim = target_->FindAnimation(step.animationKey);
		if (anim && !anim->IsPlay()) {
			// 完了コールバック
			if (step.onComplete) step.onComplete();
			AdvanceToNext();
		}
	}
}


void UIAnimationSequence::StartCurrentStep()
{
	if (currentIndex_ < 0 || currentIndex_ >= static_cast<int>(steps_.size())) return;

	const auto& step = steps_[currentIndex_];
	UIAnimationBase* anim = target_->FindAnimation(step.animationKey);
	if (anim) {
		if (step.onStart) step.onStart();
		anim->Play();
	}
	else {
		// アニメーションが見つからない場合はスキップ
		AdvanceToNext();
	}
}