#pragma once
#include <cstdint>

/* 通常攻撃タイプ */
enum class NormalAttackType {
    enNone, //!< 何もない
    enBite  //!< かみつき
};

/* 特殊能力タイプ */
enum class AbilityType {
    enNone,     //!< 何もない
    enDefault,  //!< デフォルトアビリティ                                        
    enGuard, enReflectiveGuard, enCounter,            //!< ガード関連 1～3
    enFireMagic, enFireMagic_Strong, enAbsorption,    //!< 魔法関連 4～6
    enBomb, enBomb_Decoy, enLandmine                  //!< 爆弾関連 7～9
};

/* 汎用スキルタイプ */
enum class UtilityType {
    enNone,       //!< 何もない
    enDodgeRoll   //!< 回避
};


//// 動かすものを指定する 1なら動く
//namespace UpdateGroup 
//{
//	constexpr uint32_t None = 0;		//!< 0000 0000 0000 0000
//	constexpr uint32_t Player = 1 << 0;	//!< 0000 0000 0000 0001
//	constexpr uint32_t Boss = 1 << 1;	//!< 0000 0000 0000 0010
//	constexpr uint32_t UI = 1 << 2;		//!< 0000 0000 0000 0100
//
//	constexpr uint32_t All = 0xFFFFFFFF; // 全ビット1 : すべて動くようになる
//}
