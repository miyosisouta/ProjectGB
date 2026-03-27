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


class Player;
class PlayerController;
class BossSpawner;
class Stage;
class Layout;


/**
 * サウンド管理クラス
 */
class BattleManager
{
private:
    /** 各オブジェクトのポインタ */
    Player* player_ = nullptr;
    PlayerController* playerController_ = nullptr;
    BossSpawner* boss_ = nullptr;
    Stage* stage_ = nullptr;
    Layout* layout_ = nullptr;
    SkyCube* skyCube_ = nullptr;
    
    std::unique_ptr<CameraSteering> cameraSteering_ = nullptr;
    RefCameraController gameCameraController_ = nullptr;
    RefCameraController bossEntryCameraController_ = nullptr;

	std::unique_ptr<TaskSchedulerSystem> entryBossScheduler_ = nullptr;
    bool isEndEntryBoss_ = true;


    /** 現在アクティブな対象 */
	uint32_t activeTarget_ = 0;


public:
    /** アクティブ対象を設定 */
    void SetActiveTarget(const uint32_t target);


private:
    BattleManager();
    ~BattleManager();


public:
    void Update();
    void Render(RenderContext& rc);


public:
    bool IsEndEntryBoss() const { return isEndEntryBoss_; }


private:
	bool UpdateEntryBoss();


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