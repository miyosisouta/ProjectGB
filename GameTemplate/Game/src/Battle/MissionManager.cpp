#include "stdafx.h"
#include "MissionManager.h"
#include "MissionEvent.h"
#include "MissionType.h"


namespace
{
    /* ゴリラ */
    constexpr float GOLLIRA_TARGET_TIME             = 120.0f; //!< 目標タイム
    constexpr uint8_t GOLLIRA_ABILITY_TARGET_COUNT  = 5; //!< 特殊スキル目標使用回数
    constexpr uint8_t GOLLIRA_UTILITY_TARGET_COUNT  = 1; //!< 汎用スキル目標使用回数

    /* カメ */
    constexpr float TURTLE_TARGET_TIME = 180.0f; // 目標タイム
    constexpr uint8_t TURTLE_NORMAL_ATTACK_COUNT = 10; //!< 通常攻撃の目標使用回数
    constexpr float TURTLE_GOAL_HP = 0.3f; // 目標HP残存量
}


MissionManager* MissionManager::instance_ = nullptr;


void MissionManager::NotifyEvent(const MissionEventData& ev)
{
    for (auto& mission : missions_)
    {
        mission->OnEvent(ev);
    }
}

void MissionManager::NotifyNormalAttackUsed(NormalAttackType type)
{
    MissionEventData ev;
    ev.type = MissionEventType::NormalAttackUsed;
    ev.normalAttackType = type;
    NotifyEvent(ev);
}

void MissionManager::NotifyAbilityUsed(AbilityType type)
{
    MissionEventData ev;
    ev.type = MissionEventType::AbilityUsed;
    ev.abilityType = type;
    NotifyEvent(ev);
}

void MissionManager::NotifyUtilityUsed(UtilityType type)
{
    MissionEventData ev;
    ev.type = MissionEventType::UtilityUsed;
    ev.utilityType = type;
    NotifyEvent(ev);
}

void MissionManager::NotifyBossDefeated()
{
    MissionEventData ev;
    ev.type = MissionEventType::BossDefeated;
    NotifyEvent(ev);
}


/*========================================*/
/* ボスのミッションのセットアップ */
/*========================================*/

void MissionManager::InitByBossType(BossType type)
{
    missions_.clear();

    switch (type)
    {
    case BossType::enGorilla:
    {
        SetupGorilla();
        break;
    }
    case BossType::enTurtle:
    {
        SetupTurtle();
        break;
    }
    default:
        break;
    }
}


void MissionManager::AddMission(std::unique_ptr<Mission> mission)
{
    missions_.push_back(std::move(mission));
}


void MissionManager::SetupGorilla()
{
    //TODO : jsonで設定できるようにする予定

    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaTime,                                       // IDを設定
        L"時間内討伐",                                                  // 名前を設定
        std::make_unique<ConditionKillUnderTime>(GOLLIRA_TARGET_TIME)   // クリア条件を設定
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaAbility,
        L"特殊スキル使用",
        std::make_unique<ConditionUseAbilityCount>(GOLLIRA_ABILITY_TARGET_COUNT)
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaUtility,
        L"汎用スキル使用",
        std::make_unique<ConditionUseUtilityCount>(GOLLIRA_UTILITY_TARGET_COUNT)
    ));
}


void MissionManager::SetupTurtle()
{
    //TODO : jsonで設定できるようにする予定

    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleTime,                                        // IDを設定
        L"時間内討伐",                                                  // 名前を設定
        std::make_unique<ConditionKillUnderTime>(TURTLE_TARGET_TIME)    // クリア条件を設定
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleNormalAttack,
        L"通常攻撃使用",
        std::make_unique<ConditionUseNormalAttackCount>(TURTLE_NORMAL_ATTACK_COUNT)
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleHpRate,
        L"HP30%以上でクリア",
        std::make_unique<ConditionClearWithHpRate>(TURTLE_GOAL_HP)
    ));
}



/*============================================*/
/* コンストラクタ・デストラクタ */
/*============================================*/

MissionManager::MissionManager()
{
}
MissionManager::~MissionManager()
{
}

void MissionManager::Update()
{
    for (auto& mission : missions_) 
    {
        mission->Update();
    }
}
