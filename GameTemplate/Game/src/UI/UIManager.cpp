/**
 * UIManager.cpp
 * インゲームUI管理
 */

#include "stdafx.h"
#include "UIManager.h"
#include "UIScreenManager.h"
#include "MissionMenu.h"
#include "TimerMenu.h"


UIManager::~UIManager()
{
	delete missionLayout_;
	delete timerLayout_;
	delete cutSceneLayout_;
}


void UIManager::InitMission()
{
	K2_ASSERT(!missionLayout_, "MissionLayout はすでに存在します");
	missionLayout_ = new Layout();
	missionLayout_->Initialize<MissionMenu>("Assets/ui/layout/MissionMenu.json");
}


void UIManager::InitTimer()
{
	K2_ASSERT(!timerLayout_, "TimerLayout はすでに存在します");
	timerLayout_ = new Layout();
	timerLayout_->Initialize<TimerMenu>("Assets/ui/layout/TimerMenu.json");
}


void UIManager::ReleaseCutSceneLayout()
{
	delete cutSceneLayout_;
	cutSceneLayout_ = nullptr;
}


MissionMenu* UIManager::GetMissionMenu() const
{
	return missionLayout_ ? static_cast<MissionMenu*>(missionLayout_->GetMenu()) : nullptr;
}


TimerMenu* UIManager::GetTimerMenu() const
{
	return timerLayout_ ? static_cast<TimerMenu*>(timerLayout_->GetMenu()) : nullptr;
}


void UIManager::Update()
{
	if ((updateMask_ & Mission)  && missionLayout_)  missionLayout_->Update();
	if ((updateMask_ & Timer)    && timerLayout_)    timerLayout_->Update();
	if ((updateMask_ & CutScene) && cutSceneLayout_) cutSceneLayout_->Update();
	if ( updateMask_ & Screen)                       UIScreenManager::Get().Update();

	//status_->As<PlayerStatus>()->IsExhausted();
}


void UIManager::Render(RenderContext& rc)
{
	if ((drawMask_ & Mission)  && missionLayout_)  missionLayout_->Render(rc);
	if ((drawMask_ & Timer)    && timerLayout_)    timerLayout_->Render(rc);
	if ((drawMask_ & CutScene) && cutSceneLayout_) cutSceneLayout_->Render(rc);
	if ( drawMask_ & Screen)                       UIScreenManager::Get().Render(rc);
}
