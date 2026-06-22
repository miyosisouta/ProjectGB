/**
 * CameraOptionMenu.h
 * カメラオプションメニュー
 */
#pragma once
#include "Menu.h"
#include "src/Camera/CameraManager.h"


class Layout;


class CameraOptionMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;


private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;
	UIIcon* nikukyuList[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };

	/** カメラ感度の段階(1~11段階) */
	float sensitiityStage_ = static_cast<float>(SENSITIVITY_DEFAULT_STAGE); 

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

private:
	/** カメラ感度のゲージ更新 */
	void UpdateSensitivityBar(const uint32_t gaugeId, const int volumeType);
	void UpdateMaru(const uint32_t maruId, const uint32_t wakuId, const float volume);
};