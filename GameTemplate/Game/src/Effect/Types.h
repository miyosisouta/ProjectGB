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
	enEffectKind_Dash_Wind = enEffectKind, // 0

	/* プレイヤー */
	enEffectKind_Wind_Blast_Weak,
	enEffectKind_Wind_Blast_Strong, 
	emEffectKind_Avoid,				// 3

	/* ボス */
	enEffectKind_Wind_Blast_Boss,
	enEffectKind_HitStamp,
	enEffectkind_Spin, 
	enEffectKind_DamageZone_Ring,
	enEffectKind_DamageZone_Box,	// 8
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
	EffectInformation(u"Assets/Effect/Dash_Wind.efk"),

	/* プレイヤー */
	EffectInformation(u"Assets/Effect/Wind_Blast_Weak.efk"),
	EffectInformation(u"Assets/Effect/Wind_Blast_Strong.efk"),
	EffectInformation(u"Assets/Effect/Avoid.efk"),

	/* ボス */
	EffectInformation(u"Assets/Effect/Wind_Blast_Boss.efk"),
	EffectInformation(u"Assets/Effect/HitStamp.efk"),
	EffectInformation(u"Assets/Effect/SpinAttack.efk"),
	EffectInformation(u"Assets/Effect/DamageZone_Ring.efk"),
	EffectInformation(u"Assets/Effect/DamageZone_Box.efk"),
};

