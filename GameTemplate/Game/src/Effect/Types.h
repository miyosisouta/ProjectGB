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
	enEffectKind_Wind_Blast = enEffectKind,
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
	EffectInformation(u"Assets/Effect/Wind_Blast.efk")
};

