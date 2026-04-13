/**
 * MissionMenu.h
 * ブートメニュー
 */
#pragma once
#include "Menu.h"
#include "src/Vfx/ParticleEffectRender.h"


class Layout;


class MissionMenu : public MenuBase
{
private:
	Layout* layout_;
	std::unique_ptr<TaskSchedulerSystem> missionTaskScheduler_ = nullptr;

	std::unique_ptr<UIAnimationSequence> missionSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> missionObiSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> missionMedaruSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> mission1Sequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> missionClearSequence_ = nullptr;

	// エフェクトのリスト
	std::vector<std::unique_ptr<ParticleEffectRender>> effectRenderList_;

	float elapsedTime_ = 0.0f;

	bool isCompleted_ = false;

	
public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


	bool IsCompleted() const { return isCompleted_; }
};