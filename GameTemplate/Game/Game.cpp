#include "stdafx.h"
#include "Game.h"



bool Game::Start()
{
	// @todo for test
#ifdef K2_DEBUG
	PhysicsWorld::Get().EnableDrawDebugWireFrame();
#endif
	return true;
}

void Game::Update()
{

}

void Game::Render(RenderContext& rc)
{
	
}