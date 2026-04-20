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