/**
 * TitleScene.h
 *
 * タイトルシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"


class TitleScreen;


class TitleScene : public IScene
{
public:
	Scene(TitleScene);


private:
	/** タイトル画面を表示するクラスのポインタ */
	TitleScreen* titleScreen_ = nullptr;


public:
	TitleScene();
	~TitleScene();


public:
	bool Start() override;
	void Update() override;
	bool RequestScene(uint32_t& id) override;
};

