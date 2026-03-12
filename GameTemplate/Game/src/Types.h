#pragma once
#include <cstdint>

// 動かすものを指定する 1なら動く
namespace UpdateGroup 
{
	constexpr uint32_t None = 0;		//!< 0000 0000 0000 0000
	constexpr uint32_t Player = 1 << 0;	//!< 0000 0000 0000 0001
	constexpr uint32_t Boss = 1 << 1;	//!< 0000 0000 0000 0010
	constexpr uint32_t UI = 1 << 2;		//!< 0000 0000 0000 0100

	constexpr uint32_t All = 0xFFFFFFFF; // 全ビット1 : すべて動くようになる
}
