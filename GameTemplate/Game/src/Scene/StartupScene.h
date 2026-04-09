/**
 * StartupScene.h
 * 起動画面のシーン
 * NOTE: デバッグ用でシーンの遷移を簡単にしている
 */

#pragma once
#include "IScene.h"
#include <src/Util/Crc32.h>


class Layout;

enum EnSceneKind
{
	enSceneKind_Default,
	enSceneKind_Boot = enSceneKind_Default,
	enSceneKind_Title,
	enSceneKind_Game,
	enSceneKind_Max,
};


class StartupScene : public IScene
{
	Scene(StartupScene);


private:
	/** 遷移をリクエストする先のシーンID */
	uint32_t requestSceneId_ = INVALID_SCENE_ID;
	/** シーン選択表示用 */
	FontRender sceneText_[enSceneKind_Max];
	/** どれを選択しているか分かるようにアイコンを表示 */
	SpriteRender selecterRender_;
	int selectIndex_ = enSceneKind_Default;


public:
	StartupScene();
	virtual ~StartupScene();

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;
	virtual bool RequestScene(uint32_t& id) override;
};

