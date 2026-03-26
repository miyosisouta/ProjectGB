/**
 * SoundOptionMenu.h
 * サウンドオプションメニュー
 */
#pragma once
#include "Menu.h"


class SoundOptionMenu : public MenuBase
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
	void UpdateBar(const uint32_t gaugeId, float& volume, const uint32_t volumeType);
	void UpdateMaru(const uint32_t maruId, const uint32_t wakuId, const float volume);
	void UpdateSpeaker(const uint32_t idA, const uint32_t idB, const uint32_t idC, const uint32_t idD, const float volume);
};