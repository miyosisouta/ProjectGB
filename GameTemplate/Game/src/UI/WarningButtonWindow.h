/**
 * WarningButtonWindow.h
 * 同じボタンが設定された場合に出てくる警告ウィンドウ
 */
#pragma once
#include "Menu.h"


class Layout;
class TaskSchedulerSystem;


class WarningButtonWindow : public MenuBase
{
private:
	/** 今選んでいるアクションとボタン */
	static EnGameAction selectAction_;
	static EnButton selectButton_;
	/** 重なっているアクションとボタン */
	static EnGameAction overlapAction_;
	static EnButton overlapButton_;

private:
	Layout* layout_;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

private:
	void UpdateButtonIcon(const uint32_t buttonAId, const uint32_t buttonBId, const uint32_t buttonXId, const uint32_t buttonYId, const int attackKey, const int actionKey);
	void UpdateButtonWord(const uint32_t skillId, const uint32_t DashId, const uint32_t AttackId, const int actionKey);
	void UpdateSelectedButton(const int buttonType);
	
public:
	/** キーコンフィグで設定されたボタンを取得 */
	static void SetSelectAction(EnGameAction setSelectAction)
	{
		selectAction_ = setSelectAction;
	}

	static void SetSelectButton(EnButton setSelectButton)
	{
		selectButton_ = setSelectButton;
	}

	/** 重なっている */
	static void SetOverlapAction(EnGameAction setOverlapAction)
	{
		overlapAction_ = setOverlapAction;
	}

	static void SetOverlapButton(EnButton setOverlapButton)
	{
		overlapButton_ = setOverlapButton;
	}
};