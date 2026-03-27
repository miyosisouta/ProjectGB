/**
 * GameOverMenu.h
 * ゲームオーバー時の演出等用
 */
#pragma once
#include "Menu.h"


class GameOverMenu : public MenuBase
{
private:
	std::unique_ptr<UIAnimationSequence> gameOverStampSequence_ = nullptr;
	std::unique_ptr<UIAnimationSequence> gameOverSequence_ = nullptr;
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_ = nullptr;

	/** 演出終了したか */
	bool isEnd_ = false;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


public:
	bool IsEnd() const { return isEnd_; }
};