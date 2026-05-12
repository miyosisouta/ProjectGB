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
	//クリアメダル
	auto* clearMedaru = GetUI<UIIcon>(Hash32("clear_medaru"));


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

	// ミッション１ (2分以内にクリア)
	auto* mission1 = GetUI<UIIcon>(Hash32("mission_1"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission1, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission1, Hash32("missionEventEnd"));
	mission1Sequence_ = std::make_unique<UIAnimationSequence>();
	mission1Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);

	// ミッション2 (回避できた)
	auto* mission2 = GetUI<UIIcon>(Hash32("mission_2"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission2, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission2, Hash32("missionEventEnd"));
	mission1Sequence_ = std::make_unique<UIAnimationSequence>();
	mission1Sequence_->Add(Hash32("missionEventStart")).Add(Hash32("missionEventEnd"), 5.0f);

	// ミッション3 (スキルを?回使った) 
	auto* mission3 = GetUI<UIIcon>(Hash32("mission_3"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission3, Hash32("missionEventStart"));
	UIAnimationFactory::Attach<UITranslateOffsetAnimation>(mission3, Hash32("missionEventEnd"));
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
		effectRenderList_[i]->SetPosition(Vector3(528.0f, 330, 0.0f));
		effectRenderList_[i]->EnableHotReload();
	}
}