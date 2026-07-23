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
	/** 保留中の遷移先 */
	enum class PendingLoad { None, WaitingForDog, ToBossSelect, ToTitleMenu };

	TitleBackground* titleBackground_ = nullptr; //!< タイトル背景
	bool        isRequestScene        = false; //!< シーン遷移要求フラグ
	bool        playGameMenuWasActive_ = false; //!< PlayGameMenuを開いていたか（閉じた際のスクロール再開判定用）
	bool        optionMenuWasActive_  = false; //!< OptionMenuを開いていたか（閉じた際のスクロール再開判定用）
	PendingLoad pendingLoad_          = PendingLoad::None; //!< 保留中の遷移状態
	float       loadingTimer_         = 0.0f; //!< 犬の演出・ローディングの経過時間


public:
	/** コンストラクタ */
	TitleScene();
	/** デストラクタ */
	~TitleScene();


public:
	/** スタート処理 */
	bool Start() override;
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext &rc)override;
	/** シーン遷移要求 */
	bool RequestScene(uint32_t& id) override;
};

