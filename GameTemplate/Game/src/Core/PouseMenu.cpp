#include "stdafx.h"
#include "PouseMenu.h"

#include "src/UI/UIScreenManager.h"
#include "src/UI/SoundOptionMenu.h"



PouseMenu::PouseMenu()
{
	currentMenuIndex_ = 0;
}


PouseMenu::~PouseMenu()
{
	UIScreenManager::Get().Pop();
}


bool PouseMenu::Start()
{
	menuText_.SetText(L"Option");

	return true;
}


void PouseMenu::Update()
{
	if (!isActive_) return;

	// 表示メニューの切り替え
	if (g_pad[0]->IsTrigger(enButtonUp)) {
		if (currentMenuIndex_ > 0) {
			currentMenuIndex_--;
			MoveMenu();
		}

	}
	else if(g_pad[0]->IsTrigger(enButtonDown)) {
		if (currentMenuIndex_ < 2) {
			currentMenuIndex_++;
			MoveMenu();
		}
	}

	if (g_pad[0]->IsTrigger(enButtonA)) {
		SelectMenu();
	}
	
}


void PouseMenu::Render(RenderContext& rc)
{
	if (!isActive_) return;

	menuText_.Draw(rc);
}


void PouseMenu::MoveMenu()
{
	switch (currentMenuIndex_)
	{
	case 0:
	{
		menuText_.SetText(L"Option");

		break;
	}

	case 1:
	{
		menuText_.SetText(L"To Title");


		break;
	}

	case 2:
	{
		menuText_.SetText(L"Quit");

		break;
	}

	default:
		break;
	}
}


void PouseMenu::SelectMenu()
{
	switch (currentMenuIndex_)
	{
	case 0:
	{
		UIScreenManager::Get().Push<SoundOptionMenu>("Assets/ui/layout/SoundOptionMenu.json", UITransitionMode::Push, UIScreenTransitionPreset::FadeInOut());

		break;
	}

	case 1:
	{
		isReturnTitle_ = true;

		break;
	}

	case 2:
	{
		exit(0);

		break;
	}

	default:
		break;
	}
}
