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
	enum class PendingLoad { None, ToBossSelect, ToTitleMenu };

	TitleBackground* titleBackground_ = nullptr;
	bool        isRequestScene       = false;
	bool        optionMenuWasActive_ = false;  // 設定から戻るときにスクロール再開
	PendingLoad pendingLoad_         = PendingLoad::None;
	float       loadingTimer_        = 0.0f;


public:
	TitleScene();
	~TitleScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext &rc)override;
	bool RequestScene(uint32_t& id) override;
};

