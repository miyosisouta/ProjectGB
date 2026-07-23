/**
 * GameOverMenu.h
 * ゲームオーバー時の演出等用
 */
#pragma once
#include "Menu.h"


class GameOverMenu : public MenuBase
{
private:
	std::unique_ptr<UIAnimationSequence> gameOverStampSequence_ = nullptr; //!< 落ち込むアイコンのアニメーションシーケンス
	std::unique_ptr<UIAnimationSequence> gameOverSequence_ = nullptr; //!< ゲームオーバー文字のアニメーションシーケンス
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_ = nullptr; //!< 演出タイマーの管理

	/** 演出終了したか */
	bool isEnd_ = false;

public:
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext& rc) override;
	/** UIロジックの初期化 */
	void InitializeLogic() override;


public:
	/** 演出が終了したか */
	inline bool IsEnd() const { return isEnd_; }
};