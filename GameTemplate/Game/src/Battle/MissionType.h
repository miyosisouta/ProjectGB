#pragma once

/**
 * MissionType.h
 * ミッションシステム全体で使用する列挙型・構造体の定義。
 * ロジックは持たない。
 */

/** ミッションの状態 */
enum class MissionState
{
	enActive,  //!< 進行中
	enCleared, //!< クリア済み
	enFailed   //!< 失敗済み
};

/** ミッションのUI通知タイプ */
enum class MissionUpdateType
{
	enCount,   //!< 回数カウント系（countUpdatedThisFrame_ で検知）
	enOneShot, //!< 一回で出る系（クリア・失敗時のみ）
};

/** ミッションの識別ID */
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