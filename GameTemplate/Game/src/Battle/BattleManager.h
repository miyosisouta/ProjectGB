/**
 * BattleManager.h
 * 
  * インゲーム管理
 * インゲームのゲームオブジェクトはこのクラスで管理する。
 * クラス間の情報伝達もこのクラスを介して行う。
 */


#pragma once
#include "src/Camera/CameraController.h"
#include "src/Camera/CameraSteering.h"
#include "src/Util/GameTimer.h"
#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"


class PlayerController;
class BossSpawner;
class StageManagerObject;
class Layout;


/**
 * サウンド管理クラス
 */
class BattleManager
{
private:
    enum class GameState
    {
        Entry,          // 開始前の演出
        GameStart,      // ゲームスタート演出
        Playing,        // メインのゲームプレイ中
        ResultClear,    // スコア表示・リザルト演出
        ResultOver,     // スコア表示・リザルト演出
        Shutdown        // 終了処理
    };


private:
    /** 各オブジェクトのポインタ */
    Player* player_ = nullptr;
    PlayerController* playerController_ = nullptr;
    BossSpawner* boss_ = nullptr;
    StageManagerObject* stage_ = nullptr;
    GameTimer gameTimer_;
   
    SkyCube* skyCube_ = nullptr;

    Layout* layout_ = nullptr;
    Layout* startLayout_ = nullptr;
    Layout* missionLayout_ = nullptr;
    Layout* timerLayout_ = nullptr;
    
    std::unique_ptr<CameraSteering> cameraSteering_ = nullptr;
    RefCameraController gameCameraController_ = nullptr;
    RefCameraController bossEntryCameraController_ = nullptr;

	std::unique_ptr<TaskSchedulerSystem> cutSceneScheduler_ = nullptr;
    bool isPlayingEntryBoss_ = true;
    bool isPlayingResult_ = false;

    GameState gameState_ = GameState::Entry;


    /** 現在アクティブな対象 */
	uint32_t activeTarget_ = 0;


public:
    /** アクティブ対象を設定 */
    void SetActiveTarget(const uint32_t target);

    /** ポーズ画面か否か */
    void SetPouse(const bool flg) {
        if (flg) { gameTimer_.Pause(); } // ポーズ画面を表示中ならtrueに
        else { gameTimer_.Resume(); } // それ以外ならfalse
    }

    /* ゲームタイマーを取得 */
    GameTimer* GetGameTimer() { return &gameTimer_; }
    /* プレイヤーの情報を取得 */
    float GetPlayerHPRate() {
        auto* status = player_->GetStatus()->As<ActorStatus>();
        float value = static_cast<float>(status->GetHP() / static_cast<float>(status->GetMaxHP()));
        return value;
    }

private:
    BattleManager();
    ~BattleManager();


public:
    void Update();
    void Render(RenderContext& rc);


public:
	bool IsCutScene() const
    {
        return gameState_ != GameState::Playing;
    }

    bool IsFinishedGame() const
    {
        return gameState_ == GameState::Shutdown;
	}


private:
	bool UpdateEntryBoss();
    bool UpdateResultClear();
    bool UpdateResultOver();

    void SetupEntryBossCutScene();
    void SetupStartCutScene();
    void SetupClearCutScene();
    void SetupOverCutScene();

    void ReleaseCutSceneLayout();


private:
    /** 自身のインスタンス */
    static BattleManager* myInstance_;


public:
    /** インスタンスを作る */
    static void CreateInstance()
    {
        if (!myInstance_) myInstance_ = new BattleManager();
    }


    /** インスタンスを取得 */
    static BattleManager& Get()
    {
        if(myInstance_) return *myInstance_;
    }


    /** インスタンスを破棄 */
    static void DestroyInstance()
    {
        if (myInstance_) {
            delete myInstance_;
            myInstance_ = nullptr;
        }
    }
};