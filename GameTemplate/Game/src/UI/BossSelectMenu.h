/**
 * BossSelectMenu.h
 * ボス選択メニュー
 */
#pragma once
#include "Menu.h"


class BossSelectMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;

private:
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_ = nullptr;
	UIIcon* nikukyuList_[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

private:
	void AnimationIconAttach();
	void AnimationResetIcon();
	void GolliraUpdateAnimation();
	void TurtleUpdateAnimation();

public:
	/** ゴリラを選んでいるか */
	bool IsSelectGollira() const;
	/** カメを選んでいるか */
	bool IsSelectTurtle() const;
};