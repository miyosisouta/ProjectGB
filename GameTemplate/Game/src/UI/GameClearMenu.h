/**
 * GameClearMenu.h
 * ゲームクリア時の演出等用
 */
#pragma once
#include "Menu.h"
#include "src/Vfx/ParticleEffectRender.h"


class GameClearMenu : public MenuBase
{
private:
	static constexpr int MAX_EFFECT_NUM = 4; //!< 丸エフェクトの同時再生数

	std::unique_ptr<UIAnimationSequence> gameClearSequence_ = nullptr; //!< GAME CLEARテキストのバウンスアニメーションシーケンス
	std::unique_ptr<TaskSchedulerSystem> taskSchedulerSystem_ = nullptr; //!< 演出タイムライン管理（各UIの表示タイミングを制御）

	std::vector<std::unique_ptr<ParticleEffectRender>> effectRenderList_; //!< クリアミッションの丸エフェクトリスト

	bool isEnd_ = false;         //!< 演出が終了し、シーン遷移してよい状態か
	bool isMissionShown_ = false; //!< 全ミッション結果の表示が完了し、Aボタン受付状態か

	/** indexのミッション結果カードをスライドインして背景・テキストを表示する */
	void ShowMissionBackground(int index);
	/** indexのミッション結果カードにクリア/未クリアに応じたメダルと色変化を適用する */
	void RevealMission(int index);

public:
	/** フレーム更新（タイマー・アニメーション・入力受付） */
	void Update() override;
	/** 描画（UI + 丸エフェクト） */
	void Render(RenderContext& rc) override;
	/** 演出タイムラインとアニメーションの初期設定 */
	void InitializeLogic() override;


public:
	/** 演出終了フラグを返す（trueでシーン遷移） */
	bool IsEnd() const { return isEnd_; }
};
