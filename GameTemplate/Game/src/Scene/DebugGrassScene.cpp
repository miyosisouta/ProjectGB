/**
 * DebugScene.cpp
 *
 * デバックの描画
 */

#include "stdafx.h"
#include "DebugGrassScene.h"
#include "src/Stage/StageManager.h"

namespace 
{
	constexpr uint8_t PRIORITY_STAGE = 0;
}

DebugGrassScene::DebugGrassScene()
{
}


DebugGrassScene::~DebugGrassScene()
{
	DeleteGO(stage_);
}


bool DebugGrassScene::Start()
{
	// パラメータの取得
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);
	
	// ステージ
	stage_ = NewGO<StageManagerObject>(PRIORITY_STAGE, "stage");


	return true;
}


void DebugGrassScene::Update()
{
	
}


void DebugGrassScene::Render(RenderContext& rc)
{
	
}


bool DebugGrassScene::RequestScene(uint32_t& id)
{
	return false;
}