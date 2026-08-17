/**
 * TutorialMenu.h
 * チュートリアル
 */
#pragma once
#include "Menu.h"
#include "src/Battle/TutorialManager.h"

class Layout;
class TaskSchedulerSystem;
class UIIcon;


class TutorialMenu : public MenuBase
{
private:
	Layout* layout_;

	std::unique_ptr<TaskSchedulerSystem> taskScheduler = nullptr;

	std::unique_ptr<TaskSchedulerSystem> startIconTaskScheduler_ = nullptr; //startIconのタスクスケジュラー

	UIIcon* currentIcon_ = nullptr; //!< 現在表示中のアイコン（無ければnullptr）
	bool    finishedHandled_ = false;   //!< チュートリアル終了(Free/Skip)の後始末を一度だけ行うためのフラグ

	//!< 直近でフェードインした段階。GetStage()と比較して「段階が実際に切り替わった瞬間」を検知するために使う
	TutorialManager::Stage lastFadedStage_ = TutorialManager::Stage::Move;

	//!< 前フレームの IsTransitioning() 状態。false→trueの立ち上がりエッジでフェードアウトを開始するために使う
	bool wasTransitioning_ = false;

private:
	/** Stage に対応するアイコンのハッシュ名を返す（json の "name" と対応） */
	static uint32_t GetIconHash(TutorialManager::Stage stage);

	/** 指定StageのアイコンをフェードインさせるUIAnimationを再生する */
	void FadeInText(TutorialManager::Stage stage);

	/** 現在表示中のアイコンをフェードアウトさせるUIAnimationを再生する */
	void FadeOutText();

	/** チュートリアルクリア時の枠を光らせるアニメーション */
	void FrameAnimation();

	/** Move段階が始まる前の1回だけ再生する、アイコン(back/text)の左右移動導入演出 */
	void MoveAnimation();


public:
	void Update() override;
	void Render(RenderContext& rc) override;
	void InitializeLogic() override;

};