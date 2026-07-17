#pragma once
#include <cstdint>

/** 優先度 */
enum GameObjectPriority {
    enLoadScreen = 100 //!< ロード
};

/**=====================================*/
/** 攻撃 */
/**=====================================*/

/** 通常攻撃 */
enum class NormalAttackType {
    enNone, //!< 特になし
    enBite  //!< 噛みつき
};

/** 特殊攻撃 */
enum class AbilityType {
    enNone,     //!< 特になし
    enDefault,  //!< デフォルト                                        
    enFireMagic,//!< 火
    enLandmine, //!< 地雷
};

/** 汎用攻撃 */
enum class UtilityType {
    enNone,   //!< 特になし
    enAvoid   //!< 回避
};

/*========================================*/
/* ステージ */
/*========================================*/

/* ボスの種類 */
enum class BossType 
{
    enNone = 0xFFFFFFFF, //!< 何もなし
    enGorilla,  //!< ゴリラ
    enTurtle,   //!< カメ
    enBossType_Max
};

/** ゲーム難易度 */
enum class GameModeType
{
    enNone,         //!< 特になし
    enNormal,       //!< 通常モード
    enHighAttack,   //!< 攻撃力が高いモード
    enTimeAttack    //!< HPが高い
};

/** ボスのアニメーション */
enum BossAnimID {
    enNone = -1,// 何もなし
    enAnimIdle = 0,
    enAnimRun,
    enAnimJump,
    enAnimAttack,
    enAnimHit,
    enAnimDeath,
    enAnimSpin,
    enAnimClicked,
    enAnimAntic,
    enAnimClickedWindUp, //!< 岩投げのワインドアップ（予備動作）
    enAnimJumpImpact,    //!< ヒットスタンプ着地インパクト
    enAnimNum  //!<アニメーションの最大数
};

/* アニメーションのセッティングに必要なデータの構造体 */
struct AnimSetting 
{
    std::string filePath = "";  //!< ファイルパス
    bool isLoop = false;        //!< ループさせるか
};

/** ボスのパラメータの構造体 */
struct BossParam
{
    /** ステージデータ */
    BossType stageType_ = BossType::enNone;
    GameModeType mode_ = GameModeType::enNone;
    
    /** モデルに必要なデータ */
    std::string characterKey_ = "Turtle";
    std::string modelPath_ = "";
    AnimSetting anims[enAnimNum];
    EnModelUpAxis modelAxis_ = EnModelUpAxis::enModelUpAxisZ;

    /** コリジョンの */
    float colliderRadius = 10.0f; //!< 半径
    float colliderHeight = 20.0f; //!< 高さ

    /* キャラクターのステータス */
    int maxHp_ = 0;
    int attack_ = 0;
};


// 更新・描画対象グループ (1ビット=動く/描く、0=止まる/隠れる)
namespace UpdateGroup
{
	constexpr uint32_t None   = 0;          //!< 0000 0000 (全停止)
	constexpr uint32_t Player = 1 << 0;     //!< 0000 0001
	constexpr uint32_t Boss   = 1 << 1;     //!< 0000 0010
	constexpr uint32_t UI     = 1 << 2;     //!< 0000 0100
	constexpr uint32_t Camera = 1 << 3;     //!< 0000 1000
	constexpr uint32_t Stage  = 1 << 4;     //!< 0001 0000
	constexpr uint32_t All    = 0xFFFFFFFF; //!< 全ビット1 : 全グループ有効
}