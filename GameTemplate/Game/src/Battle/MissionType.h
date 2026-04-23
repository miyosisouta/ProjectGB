#pragma once

/*
 * MissionType.h
 * ミッションシステム全体で使用する列挙型・構造体の定義。
 * ロジックは持たない。
 */

enum class MissionState
{
	enActive,
	enCleared,
	enFailed
};

enum class MissionID
{
    /* なにもない */
    enNone,

    /* ゴリラ */
    enGorillaTime,        //!< 時間内討伐
    enGorillaAbility,     //!< 特殊スキル使用
    enGorillaUtility,     //!< 汎用スキル使用

    /* カメ */
    enTurtleTime,         //!< 時間内討伐
    enTurtleNormalAttack, //!< 通常攻撃使用
    enTurtleHpRate,       //!< HP30%以上でクリア
};