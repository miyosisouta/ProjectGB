/**
 * BootMenu.h
 * ブートメニュー
 */
#pragma once
#include "Menu.h"


class Layout;


class BootMenu : public MenuBase
{
private:
	enum EnStep
	{
		PublisherLogoStart,		// パブリッシャーロゴ
		PublisherLogoUpdate,
		TeamLogoStart,			// チーム名ロゴ
		TeamLogoUpdate,
		Max,
	};


private:
	Layout* layout_;

	std::unique_ptr<UIAnimationSequence> bootImageSequence_ = nullptr;

	float elapsedTime_ = 0.0f;

	bool isCompleted_ = false;

	EnStep step_ = EnStep::PublisherLogoStart;


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;


	bool IsCompleted() const { return isCompleted_; }
};