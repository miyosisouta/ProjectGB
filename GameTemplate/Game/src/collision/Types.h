/**
 * Types.h
 * コリジョン関係の定義群
 */
#pragma once
#include "physics/CollisionAttr.h"
#include <cstdint>


struct CollisionAttribute
{
	enum Enum : uint32_t
	{
		Ground			= 1 << 0,
		Character		= 1 << 1,
		Boss			= 1 << 2,
	};
};


struct CollisionAttributeMask
{
	enum Enum : uint32_t
	{
		Ground			= CollisionAttribute::Character,
		Character		= CollisionAttribute::Ground,
		Enemy			= 1 << 2,
		All				= 0xFFFFFFFF,
	};
};


namespace ghost
{
	struct CollisionAttribute
	{
		enum Enum : uint32_t
		{
			None = 0,
			PlayerDef = 1 << 0,
			BossDef = 1 << 1,
			PlayerAtk = 1 << 2,
			BossAtk = 1 << 3,
			PlayerAvoid = 1 << 4,
			BossAvoid = 1 << 5,
		};
	};


	struct CollisionAttributeMask
	{
		enum Enum : uint32_t
		{
			Player = CollisionAttribute::BossAtk,   // ボスの攻撃だけ受ける
			Boss = CollisionAttribute::PlayerAtk, // プレイヤーの攻撃だけ受ける
			PlayerAtk = CollisionAttribute::BossDef,      // ボスのダメージ体にだけ当たる
			BossAtk = CollisionAttribute::PlayerDef,    // プレイヤーのダメージ体にだけ当たる
			All				= 0xFFFFFFFF,
		};
	};
}