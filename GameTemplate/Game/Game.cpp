#include "stdafx.h"
#include "Game.h"
#include "Src/Actor/Player.h"
#include "Src/Actor/PlayerController.h"


bool Game::Start()
{
	player_ = NewGO<Player>(0, "player");
	playerController_ = NewGO<PlayerController>(10, "playerController");
	playerController_->SetTarget(player_);
	return true;
}

void Game::Update()
{
	
}

void Game::Render(RenderContext& rc)
{
	
}