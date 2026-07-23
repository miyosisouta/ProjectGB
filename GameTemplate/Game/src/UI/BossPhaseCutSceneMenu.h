/**
 * BossPhaseCutSceneMenu.h
 * ボスHP50%/25%カットシーン演出用
 *
 * 怒りマーク(angry1/2)・疲れマーク(tired1/2/3)の表示・UIAnimation発火・
 * タイミング管理（タスクスケジューラー）をこのクラスに集約する。
 * カメラワークやボス本体（3Dモデル）の制御は BattleManager 側に残す。
 */
#pragma once
#include "Menu.h"

struct MasterBossPhaseCutSceneParameter;


class BossPhaseCutSceneMenu : public MenuBase
{
private:
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_ = nullptr; //!< 演出タイマーの管理

public:
	/** 更新処理 */
	void Update() override;
	/** UIロジックの初期化 */
	void InitializeLogic() override;

public:
	/** HP50%演出: 怒りマークを表示し、スケールパルスのループを開始する */
	void StartAngryMarks(const MasterBossPhaseCutSceneParameter& param);

	/** HP25%演出: 疲れマーク1→2→3を、弧移動+フェードアウトしながら順に表示する（呼び出し時刻を起点に自前のスケジューラーで進行する） */
	void StartTiredMarks(const MasterBossPhaseCutSceneParameter& param);

	/** マーク演出を終了し、非表示・初期状態へ戻す */
	void EndMarks(bool isPhase25, const MasterBossPhaseCutSceneParameter& param);
};
