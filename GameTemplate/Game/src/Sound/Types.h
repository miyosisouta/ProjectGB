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
	enSoundKind_Player_Walk,
	enSoundKind_Player_TakeHit,
	enSoundKind_Max = enSoundKind_Player_TakeHit,
	enSoundKind_BGM,
	enSoundKind_Title = enSoundKind_BGM,
	enSoundKind_StageSelect,
	enSoundKind_SkillSelect,
	enSoundKind_InGame,
	enSoundKind_GameClear,
	enSoundKind_GameOver,
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
	// SE
	SoundInformation("Assets/Audio/SE/Player/NormalAttack/Bite.wav"),
	SoundInformation("Assets/Audio/SE/Player/SpecialAbility/DefaultAttack.wav"),
	SoundInformation("Assets/Audio/SE/Player/Walk.wav"),
	SoundInformation("Assets/Audio/SE/Player/TakeHit.wav"),

	//BGM
	SoundInformation("Assets/Audio/BGM/Title.wav"),
	SoundInformation("Assets/Audio/BGM/StageSelect.wav"),
	SoundInformation("Assets/Audio/BGM/SkillSelect.wav"),
	SoundInformation("Assets/Audio/BGM/InGame.wav"),
	SoundInformation("Assets/Audio/BGM/GameClear.wav"),
	SoundInformation("Assets/Audio/BGM/GameOver.wav")
};

