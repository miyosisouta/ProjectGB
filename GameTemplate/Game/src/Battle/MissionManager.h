#pragma once

/**
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
private:
    std::vector<std::unique_ptr<Mission>> missions_; //!< ミッション管理用変数
    bool enabled_ = true; //!< falseの間はNotify*が全て無視される（チュートリアル中の進捗汚染防止用）


    /*========================================================*/
    /* 通知関連 */
    /*========================================================*/
private:
    /** イベントを通知 */
    void NotifyEvent(const MissionEventData& ev);

public:
    /** 通知の有効/無効を切り替える（チュートリアル中はfalseにしてNotify*を無視させる） */
    inline void SetEnabled(bool flg) { enabled_ = flg; }

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
    void InitByBossKey(const std::string& key);

private:
    /** イベントを追加 */
    void AddMission(std::unique_ptr<Mission> mission);
    /** ゴリラ戦のミッションを設定する */
    void SetupGorilla();
    /** 亀戦のミッションを設定 */
    void SetupTurtle();


private:
    /** インデックスでミッションを取得する */
    const Mission* GetMissionAt(MissionID id) const
    {
        for (const auto& mission : missions_) {
            if (mission.get()->GetID() == id) {
                return mission.get();
            }
        }
        return nullptr;
    }



    /*========================================================*/
    /* ゲッター・セッター */
    /*========================================================*/

public:
    /** 指定IDのミッションを取得 */
    /* --- ゴリラ --- */
    inline const Mission* GetGorillaTimeMission()        const { return GetMissionAt(MissionID::enGorillaTime); }
    inline const Mission* GetGorillaAbilityMission()     const { return GetMissionAt(MissionID::enGorillaAbility); }
    inline const Mission* GetGorillaUtilityMission()     const { return GetMissionAt(MissionID::enGorillaUtility); }

    /* --- カメ --- */
    inline const Mission* GetTurtleTimeMission()         const { return GetMissionAt(MissionID::enTurtleTime); }
    inline const Mission* GetTurtleNormalAttackMission() const { return GetMissionAt(MissionID::enTurtleNormalAttack); }
    inline const Mission* GetTurtleHpRateMission()       const { return GetMissionAt(MissionID::enTurtleHpRate); }

    /** ミッションの数を取得 */
    inline int GetMissionCount() const
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


public:
    /** インデックスでミッションを取得（BattleManager から直接状態を読むために公開） */
    Mission* GetMissionByIndex(int index)
    {
        if (index < 0 || index >= static_cast<int>(missions_.size())) return nullptr;
        return missions_[index].get();
    }

    /*========================================================*/
    /* コンストラクタ・デストラクタ */
    /*========================================================*/
private:
    /** コンストラクタ */
    MissionManager();
    /** デストラクタ */
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