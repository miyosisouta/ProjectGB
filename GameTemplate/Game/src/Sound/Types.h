/**
 * Types.h
 * サウンド用の定数など必要な情報を定義するファイル
 */
#pragma once
#include <string>


/** サウンドの種類 */
enum enSoundKind
{
	/** SE *************************/

	enSoundKind_SE = 0,
	/* プレイヤー */
	enSoundKind_Player_Walk = enSoundKind_SE,	//!< 移動
	enSoundKind_Player_NormalAttack,			//!< 通常攻撃
	enSoundKind_Player_SpecialAbility,			//!< 特殊攻撃
	enSoundKind_Player_Utility,					//!< 汎用スキル
	enSoundKind_Player_AttackHit,				//!< 攻撃をあてた
	enSoundKind_Player_TakeHit,					//!< 攻撃をあてられた

	/* ゴリラ */
	enSoundKind_Gorilla_Run,							//!< 走る
	enSoundKind_Gorilla_NormalAttack,					//!< 通常攻撃
	enSoundKind_Gorilla_HitStamp,						//!< ヒットスタンプ
	enSoundKind_Gorilla_Spin,							//!< 回転攻撃
	enSoundKind_Gorilla_SlowAttack,						//!< 岩を投げる攻撃
	enSoundKind_Max = enSoundKind_Gorilla_SlowAttack, //!< SEの最大数

	/* BGM */
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
	/** SE **************************/

	/* プレイヤー */
	SoundInformation("Assets/Audio/SE/Player/Walk.wav"),						// 歩き
	SoundInformation("Assets/Audio/SE/Player/NormalAttack/Bite.wav"),			// 通常攻撃
	SoundInformation("Assets/Audio/SE/Player/SpecialAbility/DefaultAttack.wav"),// 特殊スキル
	SoundInformation("Assets/Audio/SE/Player/Utility/Avoid.wav"),				// 汎用スキル
	SoundInformation("Assets/Audio/SE/Player/AttackHit.wav"),					// 攻撃をあてる
	SoundInformation("Assets/Audio/SE/Player/TakeHit.wav"),						// 攻撃をあてられた

	/** ボス */
	SoundInformation("Assets/Audio/SE/Gorilla/Footsteps.wav"),		// 走る
	SoundInformation("Assets/Audio/SE/Gorilla/NormalAttack.wav"),	// 通常攻撃
	SoundInformation("Assets/Audio/SE/Gorilla/FallAttack.wav"),		// ヒットスタンプ
	SoundInformation("Assets/Audio/SE/Gorilla/SpinAttack.wav"),		// 回転攻撃
	SoundInformation("Assets/Audio/SE/Gorilla/SlowAttack.wav"),		// 岩投げる


	/** BGM **************************/
	SoundInformation("Assets/Audio/BGM/Title.wav"),			//!< タイトル
	SoundInformation("Assets/Audio/BGM/StageSelect.wav"),	//!< ステージ選択
	SoundInformation("Assets/Audio/BGM/SkillSelect.wav"),	//!< スキル選択
	SoundInformation("Assets/Audio/BGM/InGame.wav"),		//!< インゲーム
	SoundInformation("Assets/Audio/BGM/GameClear.wav"),		//!< ゲームクリア
	SoundInformation("Assets/Audio/BGM/GameOver.wav")		//!< ゲームオーバー
};

