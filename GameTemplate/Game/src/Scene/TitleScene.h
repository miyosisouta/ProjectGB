/**
 * TitleScene.h
 *
 * タイトルシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"

class TitleBackground;

class TitleScene : public IScene
{
public:
	Scene(TitleScene);


private:
	enum class PendingLoad { None, WaitingForDog, ToBossSelect, ToTitleMenu };

	TitleBackground* titleBackground_ = nullptr;
	bool        isRequestScene        = false;
	bool        playGameMenuWasActive_ = false;
	bool        optionMenuWasActive_  = false;
	PendingLoad pendingLoad_          = PendingLoad::None;
	float       loadingTimer_         = 0.0f;


public:
	TitleScene();
	~TitleScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext &rc)override;
	bool RequestScene(uint32_t& id) override;
};

