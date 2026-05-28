/**
 * SkillSelectMenu.h
 * スキル選択メニュー
 */
#pragma once
#include "Menu.h"


class SkillSelectMenu : public MenuBase
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

public:
	/** 地雷を選んでいるか */
	bool IsSelectSkillLandMine() const;
	/** 噛みつきを選んでいるか */
	bool IsSelectSkillBite() const;
	/** 火を選んでいるか */
	bool IsSelectSkillFire() const;
};