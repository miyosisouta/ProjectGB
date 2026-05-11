/**
 * KeyConfigOptionMenu.h
 * 制限時間
 */
#pragma once
#include "Menu.h"


class KeyConfigOptionMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;

private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;
	UIIcon* nikukyuList[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void UpdateSelectedButton(const int buttonType);
	void UpdateButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int actionKey);
};