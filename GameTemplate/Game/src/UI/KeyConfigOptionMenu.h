/**
 * KeyConfigOptionMenu.h
 * 制限時間
 */
#pragma once
#include "Menu.h"


enum KeyConfigState
{
	KEY_CONFIG_STATE_SELECT,       // 選択中
	KEY_CONFIG_STATE_WAIT_ACTION,  // 入力待ち
	KEY_CONFIG_STATE_DECIDE,       // 入力完了
};


class KeyConfigOptionMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;
	
	// 状態を選択中にしておく
	KeyConfigState keyConfigState = KEY_CONFIG_STATE_SELECT;
	bool isWaiting = false;

private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;
	UIIcon* nikukyuList[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void UpdateSelectFlame();
	void UpdateSelectedButton(const int buttonType);
	void UpdateButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int actionKey);
};