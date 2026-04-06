/**
 * InGameMenu.h
 * インゲームメニュー
 */
#pragma once
#include "Menu.h"


class InGameMenu : public MenuBase
{
private:
	std::unique_ptr<UIAnimationSequence> abilitySkillIconScaleSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> bossHitHPPositionSequence_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> playerDamageScheduler_ = nullptr;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void CreatePlayerDamageScheduler();
};