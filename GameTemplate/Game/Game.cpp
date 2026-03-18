#include "stdafx.h"
#include "Game.h"
#include "Src/Actor/Player.h"
#include "Src/Actor/PlayerController.h"
#include "src/Stage/Stage.h"


// @todo for test
#include "src/UI/Layout.h"
#include "src/UI/SoundOptionMenu.h"
#include "src/UI/UIAnimationParameter.h"


namespace
{
	static Layout* layout = nullptr;
}


bool Game::Start()
{
	// @todo for test
	PhysicsWorld::Get().EnableDrawDebugWireFrame();

	player_ = NewGO<Player>(0, "player");
	playerController_ = NewGO<PlayerController>(10, "playerController");
	playerController_->SetTarget(player_);
	stage_ = NewGO<Stage>(0, "stage");


	UIAnimationParameter::Get().Load("Assets/ui/uiAnimation/UIAnimation.json");

	//layout = new Layout();
	//layout->Initialize<SoundOptionMenu>("Assets/ui/layout/SoundOptionMenu.json");

	effect_ = NewGO<EffectManagerObject>(20, "effect");
	sound_ = NewGO<SoundManagerObject>(10, "sound");
	return true;
}

void Game::Update()
{
	//layout->Update();
}

void Game::Render(RenderContext& rc)
{
	//layout->Render(rc);
}