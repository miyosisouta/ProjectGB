#pragma once

/*
 * MissionManager.h
 * ミッションシステムの管理クラス。
 * 選択されたボスの種類に応じてミッションを初期化
 * Player・Bossからのイベント通知を各ミッションへ転送
 * UIへのデータ提供もこのクラスを通して行う
 * 外部からミッションシステムへアクセスする唯一の窓口。
 */
#include "Mission.h"
#include "MissionEvent.h"


class MissionManager
{
    /*========================================================*/
    /* 通知関連 */
    /*========================================================*/
private:
    /** イベントを通知 */
    void NotifyEvent(const MissionEventData& ev);

public:
    /** 通常スキルの通知 */
    void NotifyNormalAttackUsed(NormalAttackType type);
    /** 特殊スキルの通知 */
    void NotifyAbilityUsed(AbilityType type);
    /** 汎用スキルの通知 */
    void NotifyUtilityUsed(UtilityType type);
    /** ボスを倒した通知 */
    void NotifyBossDefeated();


    /*========================================================*/
    /* セットアップ */
    /*========================================================*/

public:
    /** ボスの種類に応じてミッションを作成・初期化 */
    void InitByBossType(BossType type);

private:
    /** イベントを追加 */
    void AddMission(std::unique_ptr<Mission> mission);
    /** ゴリラ戦のミッションを設定する */
    void SetupGorilla();
    /** 亀戦のミッションを設定 */
    void SetupTurtle();


    /*========================================================*/
    /* ゲッター・セッター */
    /*========================================================*/

    /** 指定のミッションのインデックスを取得 */
    const Mission* GetMission(int index) const 
    {
        if (index < 0 || index >= static_cast<int>(missions_.size())) return nullptr;
        return missions_[index].get();
    }
    /** ミッションの数を取得 */
    int GetMissionCount() const 
    { 
        return static_cast<int>(missions_.size()); 
    }
    /** クリアしているミッションの数 */
    int GetClearedCount() const 
    {
        int count = 0;
        for (const auto& mission : missions_)
        {
            if (mission->IsCleared()) count++;
        }
        return count;
    }
    /** すべてのミッションがクリアされているか */
    bool IsAllCleared() const 
    {
        for (const auto& mission : missions_)
        {
            if (!mission->IsCleared()) return false;
        }
        return !missions_.empty();
    }


private:
    std::vector<std::unique_ptr<Mission>> missions_; //!< ミッション管理用変数


    /*========================================================*/
    /* コンストラクタ・デストラクタ */
    /*========================================================*/
private:
    MissionManager();
    ~MissionManager();


public:
    /** 更新処理 */
    void Update();


    /*========================================================*/
    /* シングルトン関連 */
    /*========================================================*/

private:
    /** 自身のインスタンス */
    static MissionManager* instance_;


public:
    /** インスタンスを作る */
    static void CreateInstance()
    {
        if (!instance_) instance_ = new MissionManager();
    }


    /** インスタンスを取得 */
    static MissionManager& Get()
    {
        if (instance_) return *instance_;
    }


    /** インスタンスを破棄 */
    static void DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};