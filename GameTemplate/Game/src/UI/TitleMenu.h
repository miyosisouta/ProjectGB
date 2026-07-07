/**
 * TitleMenu.h
 * タイトルメニュー
 */
#pragma once
#include "Menu.h"
#include "src/Vfx/ParticleEffectRender.h"


class TitleMenu : public MenuBase
{
private:
	std::unique_ptr<ParticleEffectRender> effectRender = nullptr;
	std::unique_ptr<ParticleEffectRender> effectRenderB = nullptr;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;
	std::unique_ptr<IntSelector> selector_ = nullptr;
	/** Aボタンを押したかがされているか */
	bool isAbuttonEnabled = false;

public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

	/** 「はじめる」を選んでいるか */
	bool IsSelectStat() const;
	/** 「あそびかた」を選んでいるか */
	bool IsSelectPlay() const;
	/** 「設定」を選んでいるか */
	bool IsSelectOption() const;
	/** 「おわり」を選んでいるか */
	bool IsSelectExit() const;
	/** Aボタンを押したかがされているか */
	bool IsAbuttonEnabled() const
	{
		return isAbuttonEnabled;
	}
	/** キラキラを消す */
	void StopEffect();
};