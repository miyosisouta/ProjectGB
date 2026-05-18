/**
 * BootMenu.cpp
 * ブートメニュー
 */
#include "stdafx.h"
#include "WarningButtonWindow.h"
#include "UIAnimationFactory.h"
#include "Layout.h"


void WarningButtonWindow::Update()
{


	MenuBase::Update();
}


void WarningButtonWindow::Render(RenderContext& rc)
{
	MenuBase::Render(rc);
}


void WarningButtonWindow::InitializeLogic()
{
	taskScheduler = std::make_unique<TaskSchedulerSystem>();


	
}