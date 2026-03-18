/**
 * BattleManager.h
 * 
  * インゲーム管理
 * インゲームのゲームオブジェクトはこのクラスで管理する。
 * クラス間の情報伝達もこのクラスを介して行う。
 */


#pragma once


class Player;
class PlayerController;
class Stage;
class GameCamera;


/**
 * サウンド管理クラス
 */
class BattleManager
{
private:
    /** 各オブジェクトのポインタ */
    Player* player_ = nullptr;
    PlayerController* playerController_ = nullptr;
    Stage* stage_ = nullptr;
    GameCamera* gameCamera_ = nullptr;

private:
    BattleManager();
    ~BattleManager();


public:
    void Update();


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