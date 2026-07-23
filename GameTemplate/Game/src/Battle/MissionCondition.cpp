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
    // すでにクリア済みなら以降のイベントは無視する
    if (isCleared_) return;
    // 特殊スキル使用イベント以外は無視する
    if (ev.type != MissionEventType::AbilityUsed) return;
    // 対象を絞っている場合、使用されたスキルの種別が一致しなければ無視する
    if (targetType_ != AbilityType::enNone && ev.abilityType != targetType_) return;

    // カウントを増やし、UI側にカウント更新を通知する
    count_++;
    NotifyCountUpdated();
    // 目標回数に到達したらクリア扱いにする
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
    // すでにクリア済みなら以降のイベントは無視する
    if (isCleared_) return;
    // 汎用スキル使用イベント以外は無視する
    if (ev.type != MissionEventType::UtilityUsed) return;
    // 対象を絞っている場合、使用されたスキルの種別が一致しなければ無視する
    if (targetType_ != UtilityType::enNone && ev.utilityType != targetType_) return;

    // カウントを増やし、UI側にカウント更新を通知する
    count_++;
    NotifyCountUpdated();
    // 目標回数に到達したらクリア扱いにする
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
    // すでにクリア済みなら以降のイベントは無視する
    if (isCleared_) return;
    // 通常攻撃使用イベント以外は無視する
    if (ev.type != MissionEventType::NormalAttackUsed) return;
    // 対象を絞っている場合、使用された通常攻撃の種別が一致しなければ無視する
    if (targetType_ != NormalAttackType::enNone && ev.normalAttackType != targetType_) return;

    // カウントを増やし、UI側にカウント更新を通知する
    count_++;
    NotifyCountUpdated();
    // 目標回数に到達したらクリア扱いにする
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
    // ボス撃破イベント以外は無視する（撃破の瞬間だけHP割合を判定する）
    if (ev.type != MissionEventType::BossDefeated) return;

    // プレイヤーの現在のHP割合を取得して結果判定に使う
    currentHpRate_ = BattleManager::Get().GetPlayerHPRate();

    // 必要割合以上を保っていればクリア、下回っていれば失敗
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
    // すでに撃破済みなら経過時間の更新は止める（撃破時点のタイムで判定を確定させるため）
    if (isBossDefeated_) return;
    // ゲーム全体の経過時間を毎フレーム反映する
    elapsed_ = BattleManager::Get().GetGameTimer()->GetElapsedTime();
}

void ConditionKillUnderTime::OnEvent(const MissionEventData& ev)
{
    // ボス撃破イベントを受け取ったら撃破済みフラグを立てる
    // （IsCleared/IsFailedはこのフラグと、撃破時点のelapsed_/timeLimit_の比較で判定される）
    if (ev.type == MissionEventType::BossDefeated)
    {
        isBossDefeated_ = true;
    }
}
