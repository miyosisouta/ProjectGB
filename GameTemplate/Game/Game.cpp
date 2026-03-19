#include "stdafx.h"
#include "Game.h"

//@todo for test
#include "src/UI/Layout.h"
#include "src/UI/InGameMenu.h"
#include "src/UI/UIAnimationParameter.h"


namespace
{
	static Layout* layout = nullptr;
}


bool Game::Start()
{
	// @todo for test
	PhysicsWorld::Get().EnableDrawDebugWireFrame();

	//@todo for test
	UIAnimationParameter::Get().Load("Assets/ui/uiAnimation/UIAnimation.json");

	layout = new Layout();
	layout->Initialize<InGameMenu>("Assets/ui/layout/InGameMenu.json");
	
	return true;
}

void Game::Update()
{
	// @todo for test
	layout->Update();
}

void Game::Render(RenderContext& rc)
{
	// @todo for test
	layout->Render(rc);
}