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
	enSoundKind_Player_DefaultAttack,			//!< デフォルトスペシャルスキル
	enSoundKind_Player_Landmine,				//!< 地雷
	enSoundKind_Player_FireMagic,				//!< 火魔法
	enSoundKind_Player_Utility,					//!< 汎用スキル
	enSoundKind_Player_AttackHit,				//!< 攻撃をあてた
	enSoundKind_Player_TakeHit,					//!< 攻撃をあてられた

	/* ゴリラ */
	enSoundKind_Boss_Run,							//!< 走る
	enSoundKind_Boss_NormalAttack,					//!< 通常攻撃
	enSoundKind_Boss_HitStamp,						//!< ヒットスタンプ
	enSoundKind_Boss_Spin,							//!< 回転攻撃
	enSoundKind_Boss_ThrowAttack,					//!< 岩を投げる攻撃
	enSoundKind_Boss_Thunder_Weak,					//!< レーザーを打つ弱い攻撃
	enSoundKind_Boss_Thunder_Strong,				//!< レーザーを打つ強い攻撃
	enSoundKind_Gorilla_SingleImpact,					//!< ボス登場シーンで使うドンの音
	enSoundKind_Gorilla_DoubleImpact,					//!< ボス登場シーンで使うドドンの音
	enSoundKind_Max = enSoundKind_Gorilla_DoubleImpact, //!< SEの最大数

	/** UI */
	enSoundKind_Menu_Decide,
	enSoundKind_Menu_Move,
	enSoundKind_Menu_Return,

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
	SoundInformation("Assets/Audio/SE/Player/SpecialAbility/Expload.wav"),		// 地雷スキル爆発
	SoundInformation("Assets/Audio/SE/Player/SpecialAbility/FireMagic.wav"),	// 火魔法

	SoundInformation("Assets/Audio/SE/Player/Utility/Avoid.wav"),				// 回避スキル

	SoundInformation("Assets/Audio/SE/Player/AttackHit.wav"),					// 攻撃をあてる
	SoundInformation("Assets/Audio/SE/Player/TakeHit.wav"),						// 攻撃をあてられた

	/** ボス */
	SoundInformation("Assets/Audio/SE/Enemy/Common/Footsteps.wav"),			// 走る
	SoundInformation("Assets/Audio/SE/Enemy/Common/NormalAttack.wav"),		// 通常攻撃
	SoundInformation("Assets/Audio/SE/Enemy/Common/FallAttack.wav"),		// ヒットスタンプ
	SoundInformation("Assets/Audio/SE/Enemy/Common/SpinAttack.wav"),		// 回転攻撃
	SoundInformation("Assets/Audio/SE/Enemy/Common/SlowAttack.wav"),		// 岩投げる
	SoundInformation("Assets/Audio/SE/Enemy/Common/Thunder_Weak.wav"),		// 弱い雷
	SoundInformation("Assets/Audio/SE/Enemy/Common/Thunder_Strong.wav"),	// 強い雷
	SoundInformation("Assets/Audio/SE/Enemy/Gorilla/uiTextImpactSingle.wav"),		// 登場シーンで使用するドンの音
	SoundInformation("Assets/Audio/SE/Enemy/Gorilla/uiTextImpactDouble.wav"),		// 登場シーンで使用するドドンの音

	/** UI */
	SoundInformation("Assets/Audio/SE/Menu/Decide.wav"),
	SoundInformation("Assets/Audio/SE/Menu/Move.wav"),
	SoundInformation("Assets/Audio/SE/Menu/Return.wav"),


	/** BGM **************************/
	SoundInformation("Assets/Audio/BGM/Title.wav"),			//!< タイトル
	SoundInformation("Assets/Audio/BGM/StageSelect.wav"),	//!< ステージ選択
	SoundInformation("Assets/Audio/BGM/SkillSelect.wav"),	//!< スキル選択
	SoundInformation("Assets/Audio/BGM/InGame.wav"),		//!< インゲーム
	SoundInformation("Assets/Audio/BGM/GameClear.wav"),		//!< ゲームクリア
	SoundInformation("Assets/Audio/BGM/GameOver.wav")		//!< ゲームオーバー
};

