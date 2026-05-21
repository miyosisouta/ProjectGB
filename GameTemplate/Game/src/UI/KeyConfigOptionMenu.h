/**
 * KeyConfigOptionMenu.h
 * 制限時間
 */
#pragma once
#include "Menu.h"


enum KeyConfigState
{
	KEY_CONFIG_STATE_INIT,		   // 初期化
	KEY_CONFIG_STATE_SELECT,       // 選択中
	KEY_CONFIG_STATE_WAIT_ACTION,  // 入力待ち
	KEY_CONFIG_STATE_DECIDE,       // 入力完了
	KEY_CONFIG_STATE_OVERLAP,	   // かぶった
	KEY_CONFIG_STATE_OVERLAP_DECIDE,		// 被ったことが確定した
	KEY_CONFIG_STATE_DEFAULT	   // デフォルト
};


class KeyConfigOptionMenu : public MenuBase
{
private:
	static constexpr int MAX_NIKUKYU_NUM = 4;
	
	static bool isWarningWindowCancel_;
	static bool isWarningWindowClose_;
	
	// 状態を選択中にしておく
	KeyConfigState keyConfigState_ = KEY_CONFIG_STATE_INIT;

	// 重なったときの情報を保持しておく(Action/Button 2つづつ)
	EnGameAction selectAction_;
	EnButton selectButton_;

	EnGameAction overlapAction_;
	EnButton overlapButton_;


private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_ = nullptr;
	UIIcon* nikukyuList_[MAX_NIKUKYU_NUM] = { nullptr, nullptr, nullptr, nullptr };


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


private:
	void UpdateSelectFlame();
	void UpdateSelectedButton(const int buttonType);
	void UpdateButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int actionKey);
	void InitializeButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int actionKey);

public:
	/** ボタンの設定が被った */
	bool IsButtonOverLap() const;

	/** ボタン設定中 */
	bool IsButtonSetting() const;


public:
	static void SetWarningWindowCancel(const bool isCancel) { isWarningWindowCancel_ = isCancel; }
	static bool IsWarningWindowCancel() { return isWarningWindowCancel_; }
	static void SetWarningWindowClose(const bool isClose) { isWarningWindowClose_ = isClose; }
	static bool IsWarningWindowClose() { return isWarningWindowClose_; }
};