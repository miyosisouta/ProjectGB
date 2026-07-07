/**
 * PlayGameMenu.h
 * ボス選択メニュー
 */
#pragma once
#include "Menu.h"


class PlayGameMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;

private:
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_ = nullptr;
	std::unique_ptr<UIAnimationSequence> arrowSequence_ = nullptr;
	UIIcon* nikukyuList_[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

private:
	void ArrowAnimation();
};