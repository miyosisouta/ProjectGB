/**
 * MissionMenu.cpp
 * ミッションメニュー
 */
#include "stdafx.h"
#include "MissionMenu.h"
#include "UIAnimationFactory.h"
#include "src/Util/TaskSchedulerSystem.h"
#include "Layout.h"

namespace
{
	constexpr int MAX_EFFECT_NUM = 4;
}

void MissionMenu::Update()
{
	// ミッションの背景
	auto* mission = GetUI<UIIcon>(Hash32("mission_back"));
	// ミッションの白帯
	auto* missionObi = GetUI<UIIcon>(Hash32("mission_obi"));
	// ミッションのメダル
	auto* missionMedaru = GetUI<UIIcon>(Hash32("noClear_medaru"));
	// ミッション１
	auto* mission1 = GetUI<UIIcon>(Hash32("mission_1"));
	//クリアメダル(退場)
	auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
	
	if (g_pad[0]->IsTrigger(enButtonX)) {

		missionSequence_->Play(mission);
		missionObiSequence_->Play(missionObi);
		missionMedaruSequence_->Play(missionMedaru);
		mission1Sequence_->Play(mission1);	
		missionClearSequence_->Play(clearMedaru);

		// タスクスケジューラー
		missionTaskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		// ミッション変化アニメーション
		missionTaskScheduler_->AddTimer(2.0f, [this]
			{
				// ミッションクリアの場合色が変わる
				auto* missionColor = GetUI<UIIcon>(Hash32("mission_back"));
				UIAnimationFactory::Attach<UIColorAnimation>(missionColor, Hash32("mission_color"));
				auto* colorAnimation = missionColor->FindAnimation(Hash32("mission_color"));
				colorAnimation->Play();
				
				// クリアメダル
				auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
				clearMedaru->isDraw = true;
				UIAnimationFactory::Attach<UIScaleAnimation>(clearMedaru, Hash32("mission_medaru_scale"));
				auto* scaleAnimation = clearMedaru->FindAnimation(Hash32("mission_medaru_scale"));
				scaleAnimation->Play();
			});		
		// エフェクト
		missionTaskScheduler_->AddTimer(2.5f, [&]()
			{
				for (int i = 0; i < MAX_EFFECT_NUM; i++)
				{
					effectRenderList_[i]->Play();
				}
			});
	}

	
	// タスクスケジューラー
	if (missionTaskScheduler_)
	{
		missionTaskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	}
	// シークエンス
	missionSequence_->Update(g_gameTime->GetFrameDeltaTime());
	missionObiSequence_->Update(g_gameTime->GetFrameDeltaTime());
	missionMedaruSequence_->Update(g_gameTime->GetFrameDeltaTime());
	mission1Sequence_->Update(g_gameTime->GetFrameDeltaTime());
	missionClearSequence_->Update(g_gameTime->GetFrameDeltaTime());
	
	// エフェクトの更新
	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_[i]->Update(g_gameTime->GetFrameDeltaTime());
	}

	MenuBase::Update();
}


void MissionMenu::Render(RenderContext& rc)
{
	MenuBase::Render(rc);

	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_[i]->Draw(rc);
	}

}


void MissionMenu::InitializeLogic()
{
	
	GetCanvas()->transform.localPosition = Vector3{1200.0f,330.0f,0.0f};

	// ミッション背景
	auto* mission = GetUI<UIIcon>(Hash32("mission_back"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission, Hash32("missionEventEnd"));
	missionSequence_ = std::make_unique<UIAnimationSequence>();
	missionSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);
	
	// ミッションの白帯
	auto* missionObi = GetUI<UIIcon>(Hash32("mission_obi"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionObi, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionObi, Hash32("missionEventEnd"));
	missionObiSequence_ = std::make_unique<UIAnimationSequence>();
	missionObiSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);

	// ミッションのメダル
	auto* missionMedaru = GetUI<UIIcon>(Hash32("noClear_medaru"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionMedaru, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(missionMedaru, Hash32("missionEventEnd"));
	missionMedaruSequence_ = std::make_unique<UIAnimationSequence>();
	missionMedaruSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);

	// ミッション１
	auto* mission1 = GetUI<UIIcon>(Hash32("mission_1"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission1, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission1, Hash32("missionEventEnd"));
	mission1Sequence_ = std::make_unique<UIAnimationSequence>();
	mission1Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);

	//クリアメダル(退場)
	auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(clearMedaru, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(clearMedaru, Hash32("missionEventEnd"));
	missionClearSequence_ = std::make_unique<UIAnimationSequence>();
	missionClearSequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);

	// 丸のエフェクト
	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList_.push_back(std::make_unique<ParticleEffectRender>());
		char jsonPath[] = "Assets/ui/vfx/effect_mission_maru/effect_mission_maru_1.json";
		jsonPath[54] = '1' + i;
		effectRenderList_[i]->Init(jsonPath, "Assets/ui/inGameUI/mission/maru.dds", 128.0f, 128.0f);
		effectRenderList_[i]->SetPosition(Vector3(530.0f, 330, 0.0f));
		effectRenderList_[i]->EnableHotReload();
	}


}