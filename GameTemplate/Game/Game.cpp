#include "stdafx.h"
#include "Game.h"
#include "Src/Actor/Player.h"


bool Game::Start()
{
	player_ = NewGO<Player>(0, "player");
	return true;
}

void Game::Update()
{
	
}

void Game::Render(RenderContext& rc)
{
	
}