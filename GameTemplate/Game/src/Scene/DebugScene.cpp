/**
 * DebugScene.cpp
 *
 * デバックの描画
 */

#include "stdafx.h"
#include "DebugScene.h"
#include "TitleScene.h"

#include "src/sound/SoundManager.h"
#include "src/Util/TaskSchedulerSystem.h"

#include "src/UI/Layout.h"
#include "src/UI/MissionMenu.h"


namespace
{
	constexpr int MAX_EFFECT_NUM = 4;
}


DebugScene::DebugScene()
{
}


DebugScene::~DebugScene()
{
	delete layout_;
}


bool DebugScene::Start()
{
	layout_ = new Layout;
	layout_->Initialize<MissionMenu>("Assets/ui/Layout/MissionMenu.json");

	// 星のエフェクト
	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList.push_back(std::make_unique<ParticleEffectRender>());
		char jsonPath[] = "Assets/ui/vfx/effect_mission_maru/effect_mission_maru_1.json";
		jsonPath[54] = '1' + i;
		effectRenderList[i]->Init(jsonPath, "Assets/ui/inGameUI/mission/maru.dds", 128.0f, 128.0f);
		effectRenderList[i]->SetPosition(Vector3(530.0f, 330, 0.0f));
		effectRenderList[i]->EnableHotReload();
	}
	
	
	

	return true;
}


void DebugScene::Update()
{
	layout_->Update();

	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList[i]->Update(g_gameTime->GetFrameDeltaTime());
	}

	if (g_pad[0]->IsTrigger(enButtonX))
	{
		// タスクスケジューラー
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		taskScheduler_->AddTimer(2.5f, [&]()
			{
				for (int i = 0; i < MAX_EFFECT_NUM; i++)
				{
					effectRenderList[i]->Play();
				}

			});
	}
	if (taskScheduler_) {
		taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	}

}


void DebugScene::Render(RenderContext& rc)
{
	layout_->Render(rc);

	for (int i = 0; i < MAX_EFFECT_NUM; i++)
	{
		effectRenderList[i]->Draw(rc);
	}
}


bool DebugScene::RequestScene(uint32_t& id)
{
	return false;
}