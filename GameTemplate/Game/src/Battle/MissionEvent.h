#pragma once

/**
 * MissionEvent.h
 * ミッションに通知するイベントの種別と付随データの定義。
 * Player・Bossのアクション発生時にMissionManagerへ渡すデータ構造のみを持つ。
 */

/** ミッションへ通知するイベントの種別 */
enum class MissionEventType
{
    NormalAttackUsed,   //!< 通常スキルを使用した
    AbilityUsed,        //!< 特殊スキル使用した
    UtilityUsed,        //!< 汎用スキルを使用した
    BossDefeated,       //!< ボスを倒した
};

/** ミッションへ通知するイベントデータ */
struct MissionEventData
{
    MissionEventType type;                                        //!< イベントの種別
    NormalAttackType normalAttackType = NormalAttackType::enNone; //!< 使用した通常攻撃の種別
    AbilityType abilityType = AbilityType::enNone;                //!< 使用したアビリティの種別
    UtilityType utilityType = UtilityType::enNone;                //!< 使用したユーティリティの種別
};