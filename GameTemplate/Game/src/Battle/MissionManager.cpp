#include "stdafx.h"
#include "MissionManager.h"
#include "MissionEvent.h"
#include "MissionType.h"
#include "../Core/ParameterManager.h"


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
    const auto* param = ParameterManager::Get().GetMissionParam("Gorilla");

    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaTime,
        L"時間内討伐",
        std::make_unique<ConditionKillUnderTime>(param->targetTime),
        MissionUpdateType::enOneShot,
        1   // mission_1 スロット
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaAbility,
        L"特殊スキル使用",
        std::make_unique<ConditionUseAbilityCount>(param->abilityTargetCount),
        MissionUpdateType::enCount,
        3   // mission_3 スロット（skill アイコン）
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaUtility,
        L"汎用スキル使用",
        std::make_unique<ConditionUseUtilityCount>(param->utilityTargetCount),
        MissionUpdateType::enCount,
        2   // mission_2 スロット（kaihi アイコン）
    ));
}


void MissionManager::SetupTurtle()
{
    const auto* param = ParameterManager::Get().GetMissionParam("Turtle");

    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleTime,
        L"時間内討伐",
        std::make_unique<ConditionKillUnderTime>(param->targetTime),
        MissionUpdateType::enOneShot,
        1   // turtle_mission_1 スロット
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleNormalAttack,
        L"通常攻撃使用",
        std::make_unique<ConditionUseNormalAttackCount>(param->normalAttackTargetCount),
        MissionUpdateType::enCount,
        2   // turtle_mission_2 スロット
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleHpRate,
        L"HP30%以上でクリア",
        std::make_unique<ConditionClearWithHpRate>(param->goalHpRate),
        MissionUpdateType::enOneShot,
        3   // turtle_mission_3 スロット
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
