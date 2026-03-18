/**
 * Types.h
 * サウンド用の定数など必要な情報を定義するファイル
 */
#pragma once
#include <string>


/** サウンドの種類 */
enum enSoundKind
{
	enSoundKind_SE = 0,
	enSoundKind_Player_NormalAttack = enSoundKind_SE,
	enSoundKind_Player_Bite,
	enSoundKind_SE_Max,
};


/** サウンドの情報の構造体 */
struct SoundInformation
{
	std::string assetPath;
	//
	SoundInformation(const std::string& path) : assetPath(path) {}
};


/** 情報を保持 */
static SoundInformation soundInformation[enSoundKind_SE_Max] =
{
	//BGM
	

	// SE
	SoundInformation("Assets/Audio/SE/Player/NormalAttack/Bite.wav"),
	SoundInformation("Assets/Audio/SE/Player/SpecialAbility/DefaultAttack.wav")
};

