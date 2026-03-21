/**
 * TitleScene.h
 *
 * タイトルシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"


class TitleScene : public IScene
{
public:
	Scene(TitleScene);


private:
	/** タイトル画面を表示するクラスのポインタ */
	bool isRequestScene = false;


public:
	TitleScene();
	~TitleScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext &rc)override;
	bool RequestScene(uint32_t& id) override;
};

