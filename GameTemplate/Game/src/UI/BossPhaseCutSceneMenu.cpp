/**
 * BossPhaseCutSceneMenu.cpp
 * ボスHP50%/25%カットシーン演出用
 */
#include "stdafx.h"
#include "BossPhaseCutSceneMenu.h"
#include "UIAnimationFactory.h"
#include "src/CharacterDataBase.h"


void BossPhaseCutSceneMenu::Update()
{
	const float deltaTime = g_gameTime->GetFrameDeltaTime();
	taskScheduler_->Update(deltaTime);

	MenuBase::Update();
}


void BossPhaseCutSceneMenu::InitializeLogic()
{
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// 怒り/疲れマークの表示位置はボスの種類ごとに立ち位置・高さが違うため、JSON側のYオフセットを反映する
	MasterBossPhaseCutSceneParameter defaultIconParam;
	const std::string stageKey = CharacterDataBase::Get().GetStageKey();
	const auto* iconParam = ParameterManager::Get().GetBossPhaseCutSceneParam(stageKey);
	if (!iconParam) { iconParam = &defaultIconParam; }

	// 怒りマークのスケールパルス用アニメーションをアタッチしておく（再生自体はStartAngryMarks()で開始する）
	if (auto* angry1 = GetUI<UIIcon>(Hash32("angry1"))) {
		angry1->transform.localPosition.y += iconParam->iconOffsetY;
		UIAnimationFactory::Attach<UIScaleAnimation>(angry1, Hash32("angry1_scaleDown"));
		UIAnimationFactory::Attach<UIScaleAnimation>(angry1, Hash32("angry1_scaleUp"));
	}
	if (auto* angry2 = GetUI<UIIcon>(Hash32("angry2"))) {
		angry2->transform.localPosition.y += iconParam->iconOffsetY;
		UIAnimationFactory::Attach<UIScaleAnimation>(angry2, Hash32("angry2_scaleDown"));
		UIAnimationFactory::Attach<UIScaleAnimation>(angry2, Hash32("angry2_scaleUp"));
	}

	// 疲れマークの弧移動・フェードアウト用アニメーションをアタッチしておく（再生自体はStartTiredMarks()で開始する）
	// 表示位置は怒りマークと同様、ボスの種類ごとのYオフセットに加え、外側へのXオフセットも反映する。
	// 弧移動はUITranslateOffsetAnimation（アタッチ時点の位置からの相対オフセット）にしているため、
	// 先に位置をオフセットしてからAttachすることで、ボスの種類が変わっても同じ相対量でオフセット後の位置を基準に動く
	if (auto* tired1 = GetUI<UIIcon>(Hash32("tired1"))) {
		tired1->transform.localPosition.y += iconParam->iconOffsetY;
		tired1->transform.localPosition.x += iconParam->tiredIconOffsetX;
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(tired1, Hash32("tired1_arcRise"));
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(tired1, Hash32("tired1_arcDrift"));
		UIAnimationFactory::Attach<UIColorAnimation>(tired1, Hash32("tired1_fadeOut"));
	}
	if (auto* tired2 = GetUI<UIIcon>(Hash32("tired2"))) {
		tired2->transform.localPosition.y += iconParam->iconOffsetY;
		tired2->transform.localPosition.x -= iconParam->tiredIconOffsetX; // tired2は反対側(-X)にあるため逆方向に外側へずらす
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(tired2, Hash32("tired2_arcRise"));
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(tired2, Hash32("tired2_arcDrift"));
		UIAnimationFactory::Attach<UIColorAnimation>(tired2, Hash32("tired2_fadeOut"));
	}
	if (auto* tired3 = GetUI<UIIcon>(Hash32("tired3"))) {
		tired3->transform.localPosition.y += iconParam->iconOffsetY;
		tired3->transform.localPosition.x += iconParam->tiredIconOffsetX;
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(tired3, Hash32("tired3_arcRise"));
		UIAnimationFactory::Attach<UITranslateOffsetAnimation>(tired3, Hash32("tired3_arcDrift"));
		UIAnimationFactory::Attach<UIColorAnimation>(tired3, Hash32("tired3_fadeOut"));
	}
}


void BossPhaseCutSceneMenu::StartAngryMarks(const MasterBossPhaseCutSceneParameter& param)
{
	// 怒りマークを表示する
	if (auto* icon = GetUI<UIIcon>(Hash32("angry1"))) { icon->isDraw = true; }
	if (auto* icon = GetUI<UIIcon>(Hash32("angry2"))) { icon->isDraw = true; }

	// 怒りマークのスケールパルスを開始する（縮小は速く、拡大はやや遅め。
	// 2枚目はicon2PulseDelay秒ずらして開始することで、2枚がズレて動いて見えるようにする）
	const float iconScaleDownDuration = param.iconScaleDownDuration;
	const float iconScaleUpDuration   = param.iconScaleUpDuration;
	const float iconPulsePeriod       = iconScaleDownDuration + iconScaleUpDuration;
	const float icon2PulseDelay       = param.icon2PulseDelay;

	const int pulseSeq1 = taskScheduler_->CreateLoopSequence(iconPulsePeriod);
	taskScheduler_->AddLoopTimer(pulseSeq1, 0.0f, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("angry1"))) {
				if (auto* anim = icon->FindAnimation(Hash32("angry1_scaleDown"))) { anim->Clear(); anim->Play(); }
			}
		});
	taskScheduler_->AddLoopTimer(pulseSeq1, iconScaleDownDuration, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("angry1"))) {
				if (auto* anim = icon->FindAnimation(Hash32("angry1_scaleUp"))) { anim->Clear(); anim->Play(); }
			}
		});

	const int pulseSeq2 = taskScheduler_->CreateLoopSequence(iconPulsePeriod);
	taskScheduler_->AddLoopTimer(pulseSeq2, icon2PulseDelay, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("angry2"))) {
				if (auto* anim = icon->FindAnimation(Hash32("angry2_scaleDown"))) { anim->Clear(); anim->Play(); }
			}
		});
	taskScheduler_->AddLoopTimer(pulseSeq2, icon2PulseDelay + iconScaleDownDuration, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("angry2"))) {
				if (auto* anim = icon->FindAnimation(Hash32("angry2_scaleUp"))) { anim->Clear(); anim->Play(); }
			}
		});
}


void BossPhaseCutSceneMenu::StartTiredMarks(const MasterBossPhaseCutSceneParameter& param)
{
	// 疲れマーク1→2→3の順にtiredStaggerInterval秒間隔で開始する。
	// 各マークは弧の上昇(tiredArcRiseDuration秒)→横方向への移動(UIAnimation.json側のtiredN_arcDriftの秒数)の2段階で移動しつつ、
	// 同時に透明度もフェードアウトさせる（BossPhaseCutSceneParameter.jsonで調整）
	const float tiredArcRiseDuration = param.tiredArcRiseDuration;
	const float tiredStaggerInterval = param.tiredStaggerInterval;

	// 疲れマーク1（0秒後に開始。アニメーション開始と同時に描画も始める）
	taskScheduler_->AddTimer(0.0f, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("tired1"))) {
				icon->isDraw = true;
				if (auto* anim = icon->FindAnimation(Hash32("tired1_arcRise"))) { anim->Clear(); anim->Play(); }
				if (auto* anim = icon->FindAnimation(Hash32("tired1_fadeOut"))) { anim->Clear(); anim->Play(); }
			}
		});
	taskScheduler_->AddTimer(tiredArcRiseDuration, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("tired1"))) {
				if (auto* anim = icon->FindAnimation(Hash32("tired1_arcDrift"))) { anim->Clear(); anim->Play(); }
			}
		});

	// 疲れマーク2（tiredStaggerInterval秒後に開始。アニメーション開始と同時に描画も始める）
	taskScheduler_->AddTimer(tiredStaggerInterval, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("tired2"))) {
				icon->isDraw = true;
				if (auto* anim = icon->FindAnimation(Hash32("tired2_arcRise"))) { anim->Clear(); anim->Play(); }
				if (auto* anim = icon->FindAnimation(Hash32("tired2_fadeOut"))) { anim->Clear(); anim->Play(); }
			}
		});
	taskScheduler_->AddTimer(tiredStaggerInterval + tiredArcRiseDuration, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("tired2"))) {
				if (auto* anim = icon->FindAnimation(Hash32("tired2_arcDrift"))) { anim->Clear(); anim->Play(); }
			}
		});

	// 疲れマーク3（tiredStaggerInterval*2秒後に開始。アニメーション開始と同時に描画も始める）
	taskScheduler_->AddTimer(tiredStaggerInterval * 2.0f, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("tired3"))) {
				icon->isDraw = true;
				if (auto* anim = icon->FindAnimation(Hash32("tired3_arcRise"))) { anim->Clear(); anim->Play(); }
				if (auto* anim = icon->FindAnimation(Hash32("tired3_fadeOut"))) { anim->Clear(); anim->Play(); }
			}
		});
	taskScheduler_->AddTimer(tiredStaggerInterval * 2.0f + tiredArcRiseDuration, [this]()
		{
			if (auto* icon = GetUI<UIIcon>(Hash32("tired3"))) {
				if (auto* anim = icon->FindAnimation(Hash32("tired3_arcDrift"))) { anim->Clear(); anim->Play(); }
			}
		});
}


void BossPhaseCutSceneMenu::EndMarks(const bool isPhase25, const MasterBossPhaseCutSceneParameter& param)
{
	if (isPhase25)
	{
		// 疲れマークを非表示に戻す
		if (auto* icon = GetUI<UIIcon>(Hash32("tired1"))) { icon->isDraw = false; }
		if (auto* icon = GetUI<UIIcon>(Hash32("tired2"))) { icon->isDraw = false; }
		if (auto* icon = GetUI<UIIcon>(Hash32("tired3"))) { icon->isDraw = false; }
	}
	else
	{
		// スケールパルスを止め、次に表示する時のためにベースの大きさへ戻しておく
		if (auto* icon = GetUI<UIIcon>(Hash32("angry1"))) {
			icon->isDraw = false;
			if (auto* anim = icon->FindAnimation(Hash32("angry1_scaleDown"))) { anim->Stop(); }
			if (auto* anim = icon->FindAnimation(Hash32("angry1_scaleUp")))   { anim->Stop(); }
			icon->transform.localScale = Vector3(param.angry1BaseScale, param.angry1BaseScale, param.angry1BaseScale);
		}
		if (auto* icon = GetUI<UIIcon>(Hash32("angry2"))) {
			icon->isDraw = false;
			if (auto* anim = icon->FindAnimation(Hash32("angry2_scaleDown"))) { anim->Stop(); }
			if (auto* anim = icon->FindAnimation(Hash32("angry2_scaleUp")))   { anim->Stop(); }
			icon->transform.localScale = Vector3(param.angry2BaseScale, param.angry2BaseScale, param.angry2BaseScale);
		}
	}
}
