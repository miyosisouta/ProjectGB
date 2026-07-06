/**
 * Types.h
 * エフェクト用の定数など必要な情報を定義するファイル
 */
#pragma once
#include <string>


/** エフェクトの種類 */
enum enEffectKind
{
	enEffectKind = 0,
	/* 共通エフェクト */
	enEffectKind_Dash_Wind = enEffectKind, 
	enEffectKind_Expload,			// 1

	/* プレイヤー */
	enEffectKind_Wind_Blast_Weak,
	enEffectKind_Wind_Blast_Strong,
	enEffectKind_Fire_Bress,
	enEffectKind_Avoid,				
	enEffectKind_AttackHitPlayer,	// 6

	/* ボス */
	enEffectKind_Wind_Blast_Boss,
	enEffectKind_HitStamp_Smoke,
	enEffectKind_HitStamp_ShockWave,
	enEffectkind_Spin,
	enEffectKind_Raser,
	enEffectKind_DamageZone_Ring,
	enEffectKind_DamageZone_Box,
	enEffectKind_AttackHitBoss,		// 14

	/* 感情システム */
	enEffectKind_Buff,				// 15 強気化時
	enEffectKind_Debuff,			// 16 動揺時

	enEffectKind_Max
};


/** エフェクトの情報の構造体 */
struct EffectInformation
{
	const char16_t* assetPath;
	//
	EffectInformation(const char16_t* path) : assetPath(path) {}
};


/** 情報を保持 */
static EffectInformation effectInformation[enEffectKind_Max] =
{
	/* 共通エフェクト */
	EffectInformation(u"Assets/Effect/Dash_Wind.efk"),			// 0
	EffectInformation(u"Assets/Effect/Expload.efk"),

	/* プレイヤー */
	EffectInformation(u"Assets/Effect/Wind_Blast_Weak.efk"),
	EffectInformation(u"Assets/Effect/Wind_Blast_Strong.efk"),
	EffectInformation(u"Assets/Effect/Fire_Bress.efk"),
	EffectInformation(u"Assets/Effect/Avoid.efk"),
	EffectInformation(u"Assets/Effect/HitPlayer.efk"),			// 6

	/* ボス */
	EffectInformation(u"Assets/Effect/Wind_Blast_Boss.efk"),
	EffectInformation(u"Assets/Effect/HitStamp_Smoke.efk"),
	EffectInformation(u"Assets/Effect/HitStamp_ShockWave.efk"),
	EffectInformation(u"Assets/Effect/SpinAttack.efk"),
	EffectInformation(u"Assets/Effect/Thunder.efk"),
	EffectInformation(u"Assets/Effect/DamageZone_Ring.efk"),
	EffectInformation(u"Assets/Effect/DamageZone_Box.efk"),
	EffectInformation(u"Assets/Effect/HitBoss.efk"),			// 14

	/* 感情システム */
	EffectInformation(u"Assets/Effect/Buff.efk"),				// 15
	EffectInformation(u"Assets/Effect/Debuff.efk"),			// 16
};

