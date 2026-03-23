#pragma once
#include <cstdint>

/** ゲームオブジェクトの優先度（priority） */
enum GameObjectPriority {
    enLoadScreen = 100 //!< ロード画面
};

/*========================================*/
/* プレイヤーのスキル情報 */
/*========================================*/

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

/*========================================*/
/* ボスの情報 */
/*========================================*/

/* ボスの種類 */
enum class BossType 
{
    enNone = 0xFFFFFFFF, //!< 何もない
    enGorilla,  //!< ゴリラ
    enKangaroo, //!< カンガルー
    enTurtle,   //!< カメ
    enBossType_Max
};

/* ステージのモード */
enum class GameModeType
{
    enNone,         //!< 何もない
    enNormal,       //!< 通常モード
    enHighAttack,   //!< 攻撃力特化モード
    enTimeAttack    //!< 防御力特化（タイムアタック）モード
};

/* アニメーションのID（配列の番号として使う） */
enum BossAnimID {
    enNone = -1,// 何もない
    enAnimIdle = 0,
    enAnimRun,
    enAnimJump,
    enAnimAttack,
    enAnimHit,
    enAnimDeath,
    enAnimSpin,
    enAnimClicked,
    enAnimNum  //!< アニメーションの総数（自動的に7になる）
};

/* 1つのアニメーションの「パス」と「ループ設定」をまとめる */
struct AnimSetting 
{
    std::string filePath = "";
    bool isLoop = false;
};

/* ボスに必要なパラメータ */
struct BossParam
{
    /* ステージ選択で選ばれた内容 */
    BossType stageType_ = BossType::enNone;
    GameModeType mode_ = GameModeType::enNone;
    
    /* モデル */
    std::string modelPath_ = "";
    AnimSetting anims[enAnimNum];

    /* コリジョン */
    float colliderRadius = 10.0f; //!< 当たり判定（カプセルやスフィア）の半径
    float colliderHeight = 20.0f; //!< 当たり判定の高さ

    /* ステータス */
    int maxHp_ = 0;
    int attack_ = 0;
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
