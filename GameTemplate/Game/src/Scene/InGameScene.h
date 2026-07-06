/**
 * InGameScene.h
 *
 * インゲームシーンの遷移
 */

#pragma once
#include "src/Scene/IScene.h"

class Layout;
class PouseMenu;

class InGameScene : public IScene
{
private:
	Layout* layout_                  = nullptr;
	PouseMenu* pouseMenu_ = nullptr;


	/** 現在動かす対象（player,UI,Stage,Boss) */
	uint32_t activeTarget_ = 0;
	/** ポーズメニューの開閉状態 */
	bool isOpenPauseMenu_ = false;


public:
	/** 更新の対象物を取得 */
	inline const uint32_t GetActiveTarget() const { return activeTarget_; }


public:
	Scene(InGameScene);


public:
	InGameScene();
	~InGameScene();


public:
	bool Start() override;
	void Update() override;
	void Render(RenderContext& rc);
	bool RequestScene(uint32_t& id) override;
};

