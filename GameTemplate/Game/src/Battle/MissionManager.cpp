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
        MissionID::enGorillaTime,                                           // IDを設定
        L"時間内討伐",                                                      // 名前を設定
        std::make_unique<ConditionKillUnderTime>(param->targetTime)         // クリア条件を設定
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaAbility,
        L"特殊スキル使用",
        std::make_unique<ConditionUseAbilityCount>(param->abilityTargetCount)
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaUtility,
        L"汎用スキル使用",
        std::make_unique<ConditionUseUtilityCount>(param->utilityTargetCount)
    ));
}


void MissionManager::SetupTurtle()
{
    const auto* param = ParameterManager::Get().GetMissionParam("Turtle");

    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleTime,                                            // IDを設定
        L"時間内討伐",                                                      // 名前を設定
        std::make_unique<ConditionKillUnderTime>(param->targetTime)         // クリア条件を設定
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleNormalAttack,
        L"通常攻撃使用",
        std::make_unique<ConditionUseNormalAttackCount>(param->normalAttackTargetCount)
    ));
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleHpRate,
        L"HP30%以上でクリア",
        std::make_unique<ConditionClearWithHpRate>(param->goalHpRate)
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
