#include "stdafx.h"
#include "MissionCondition.h"

/*=============================================*/
/* 特殊スキルの使用回数の条件 */
/*=============================================*/

ConditionUseAbilityCount::ConditionUseAbilityCount(int requiredCount, AbilityType targetType)
	:required_(requiredCount), targetType_(targetType)
{
}

void ConditionUseAbilityCount::OnEvent(const MissionEventData& ev)
{
    if (isCleared_) return;
     if (ev.type != MissionEventType::AbilityUsed) return;
     if (targetType_ != AbilityType::enNone && ev.abilityType != targetType_) return;

    count_++;
    NotifyCountUpdated();
    if (count_ >= required_) { isCleared_ = true; }
}



/*=============================================*/
/* 汎用スキルの使用回数の条件 */
/*=============================================*/

ConditionUseUtilityCount::ConditionUseUtilityCount(int requiredCount, UtilityType targetType)
    :required_(requiredCount),targetType_(targetType)
{
}

void ConditionUseUtilityCount::OnEvent(const MissionEventData& ev)
{
    if (isCleared_) return;
    if (ev.type != MissionEventType::UtilityUsed) return;
    if (targetType_ != UtilityType::enNone && ev.utilityType != targetType_) return;

    count_++;
    NotifyCountUpdated();
    if (count_ >= required_) { isCleared_ = true; }
}



/*=============================================*/
/* 通常攻撃の使用回数の条件 */
/*=============================================*/

ConditionUseNormalAttackCount::ConditionUseNormalAttackCount(int requiredCount, NormalAttackType targetType)
    : required_(requiredCount), targetType_(targetType)
{
}

void ConditionUseNormalAttackCount::OnEvent(const MissionEventData& ev)
{
    if (isCleared_) return;
    if (ev.type != MissionEventType::NormalAttackUsed) return;
    if (targetType_ != NormalAttackType::enNone && ev.normalAttackType != targetType_) return;

    count_++;
    NotifyCountUpdated();
    if (count_ >= required_) { isCleared_ = true; }
}



/*=============================================*/
/* 残りHP割合以上でボスを倒す条件 */
/*=============================================*/

ConditionClearWithHpRate::ConditionClearWithHpRate(float minHpRate)
    : minHpRate_(minHpRate)
{
}

void ConditionClearWithHpRate::OnEvent(const MissionEventData& ev)
{
    if (ev.type != MissionEventType::BossDefeated) return;

    // プレイヤーの現在のHPを取得
    currentHpRate_ = BattleManager::Get().GetPlayerHPRate();

    if (currentHpRate_ >= minHpRate_)
    {
        isCleared_ = true;
    }
    else
    {
        isFailed_ = true;
    }
}



/*=============================================*/
/* 制限時間内に撃破 */
/*=============================================*/

ConditionKillUnderTime::ConditionKillUnderTime(float timeLimitSec)
    : timeLimit_(timeLimitSec)
{
}

void ConditionKillUnderTime::Update()
{
    if (isBossDefeated_) return;
    elapsed_ = BattleManager::Get().GetGameTimer()->GetElapsedTime();
}

void ConditionKillUnderTime::OnEvent(const MissionEventData& ev)
{
    if (ev.type == MissionEventType::BossDefeated)
    {
        isBossDefeated_ = true;
    }
}
