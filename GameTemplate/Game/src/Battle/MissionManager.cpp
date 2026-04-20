#include "stdafx.h"
#include "MissionManager.h"
#include "MissionEvent.h"
#include "MissionType.h"


namespace
{
    constexpr float TARGET_TIME = 20.0f; //!< 目標タイム
    constexpr uint8_t ABILITY_TARGET_COUNT = 5; //!< 特殊スキル目標使用回数
    constexpr uint8_t UTILITY_TARGET_COUNT = 1; //!< 汎用スキル目標使用回数
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
    AddMission(std::make_unique<Mission>(
        L"時間内討伐",
        std::make_unique<ConditionKillUnderTime>(TARGET_TIME)
    ));
    AddMission(std::make_unique<Mission>(
        L"特殊スキル使用",
        std::make_unique<ConditionUseAbilityCount>(ABILITY_TARGET_COUNT)
    ));
    AddMission(std::make_unique<Mission>(
        L"汎用スキル使用",
        std::make_unique<ConditionUseUtilityCount>(UTILITY_TARGET_COUNT)
    ));
}


void MissionManager::SetupTurtle()
{
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
