#include "stdafx.h"
#include "PouseMenu.h"

#include "src/UI/Layout.h"
#include "src/UI/PauseMenu.h"



PouseMenu::PouseMenu()
{
}


PouseMenu::~PouseMenu()
{
	if (layout_) {
		delete layout_;
		layout_ = nullptr;
	}
}


bool PouseMenu::Start()
{
	layout_ = new Layout();
	layout_->Initialize<PauseMenu>("Assets/ui/layout/PauseMenu.json");

	return true;
}


void PouseMenu::Update()
{
	if (!isActive_) return;
	
	auto* menu = static_cast<PauseMenu*>(layout_->GetMenu());
	isReturnTitle_ = menu->IsReturnTitle();

	layout_->Update();
}


void PouseMenu::Render(RenderContext& rc)
{
	if (!isActive_) return;

	layout_->Render(rc);
}