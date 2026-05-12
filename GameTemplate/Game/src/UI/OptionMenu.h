/**
 * OptionMenu.h
 * オプションメニュー
 */
#pragma once
#include "Menu.h"


class OptionMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;


private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;
	std::unique_ptr<UIAnimationSequence> controllerSequence_ = nullptr;

	UIIcon* nikukyuList[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void SoundIconAnimation();
	void ControllerIconAnimation();
	void CameraIconAnimation();
	void ResetIcon();
	void AnimationIconAttach();
};