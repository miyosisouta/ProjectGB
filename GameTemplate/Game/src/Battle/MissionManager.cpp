#include "stdafx.h"
#include "MissionManager.h"
#include "MissionEvent.h"
#include "MissionType.h"
#include "../Core/ParameterManager.h"


MissionManager* MissionManager::instance_ = nullptr;


void MissionManager::NotifyEvent(const MissionEventData& ev)
{
    // チュートリアル中など無効化されている間は、練習用の入力をミッション進捗に反映しない
    if (!enabled_) { return; }

    // 保持している全ミッションへ同じイベントを配信する（該当しないミッションは各Condition側で無視される）
    for (auto& mission : missions_)
    {
        mission->OnEvent(ev);
    }
}

void MissionManager::NotifyNormalAttackUsed(NormalAttackType type)
{
    // 通常攻撃使用イベントを組み立てて配信する
    MissionEventData ev;
    ev.type = MissionEventType::NormalAttackUsed;
    ev.normalAttackType = type;
    NotifyEvent(ev);
}

void MissionManager::NotifyAbilityUsed(AbilityType type)
{
    // 特殊スキル使用イベントを組み立てて配信する
    MissionEventData ev;
    ev.type = MissionEventType::AbilityUsed;
    ev.abilityType = type;
    NotifyEvent(ev);
}

void MissionManager::NotifyUtilityUsed(UtilityType type)
{
    // 汎用スキル使用イベントを組み立てて配信する
    MissionEventData ev;
    ev.type = MissionEventType::UtilityUsed;
    ev.utilityType = type;
    NotifyEvent(ev);
}

void MissionManager::NotifyBossDefeated()
{
    // ボス撃破イベントを組み立てて配信する（HPレート条件・時間内討伐条件などがこれで確定する）
    MissionEventData ev;
    ev.type = MissionEventType::BossDefeated;
    NotifyEvent(ev);
}


/*========================================*/
/* ボスのミッションのセットアップ */
/*========================================*/

void MissionManager::InitByBossKey(const std::string& key)
{
    // 前回のミッションが残っていると重複するので、まず全消去する
    missions_.clear();

    // 対戦するボスの種類に応じて、そのボス専用のミッション一式を構築する
    if (key == "Gorilla") {
        SetupGorilla();
    }
    else if (key == "Turtle") {
        SetupTurtle();
    }
    else {
        // 未対応のボス種別はミッションなしのまま
        K2_ASSERT(false, ("MissionManager: 未対応のボスkeyです（key='" + key + "'）\n").c_str());
    }
}


void MissionManager::AddMission(std::unique_ptr<Mission> mission)
{
    // 所有権を受け取ってミッション一覧に追加する
    missions_.push_back(std::move(mission));
}


void MissionManager::SetupGorilla()
{
    // ゴリラ用のミッションパラメータ（目標タイム・目標回数など）をJSONから取得
    const auto* param = ParameterManager::Get().GetMissionParam("Gorilla");

    // ミッション1: 制限時間内に討伐（ワンショット判定、mission_1スロットに表示）
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaTime,
        L"時間内討伐",
        std::make_unique<ConditionKillUnderTime>(param->targetTime),
        MissionUpdateType::enOneShot,
        1   // mission_1 スロット
    ));
    // ミッション2: 特殊スキルを目標回数使用（カウント判定、mission_2スロット=skillアイコンに表示）
    AddMission(std::make_unique<Mission>(
        MissionID::enGorillaAbility,
        L"特殊スキル使用",
        std::make_unique<ConditionUseAbilityCount>(param->abilityTargetCount),
        MissionUpdateType::enCount,
        3   // mission_3 スロット（skill アイコン）
    ));
    // ミッション3: 汎用スキル（回避）を目標回数使用（カウント判定、mission_3スロット=kaihiアイコンに表示）
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
    // カメ用のミッションパラメータ（目標タイム・目標回数・目標HP割合など）をJSONから取得
    const auto* param = ParameterManager::Get().GetMissionParam("Turtle");

    // ミッション1: 制限時間内に討伐（ワンショット判定、turtle_mission_1スロットに表示）
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleTime,
        L"時間内討伐",
        std::make_unique<ConditionKillUnderTime>(param->targetTime),
        MissionUpdateType::enOneShot,
        1   // turtle_mission_1 スロット
    ));
    // ミッション2: 通常攻撃を目標回数使用（カウント判定、turtle_mission_2スロットに表示）
    AddMission(std::make_unique<Mission>(
        MissionID::enTurtleNormalAttack,
        L"通常攻撃使用",
        std::make_unique<ConditionUseNormalAttackCount>(param->normalAttackTargetCount),
        MissionUpdateType::enCount,
        2   // turtle_mission_2 スロット
    ));
    // ミッション3: 残りHP30%以上を保ったままボスを倒す（ワンショット判定、turtle_mission_3スロットに表示）
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
    // 保持している全ミッションを1フレーム分進める（時間経過で進行する条件はここで判定される）
    for (auto& mission : missions_)
    {
        mission->Update();
    }
}
