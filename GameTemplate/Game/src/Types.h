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

/**
 * ボスの種類とCharacterMaster.json等のキー文字列の対応一覧。
 * 新しいボスを追加する時は、このリストに1行足すだけでよい
 * （BossType enumの値と、BossType↔key文字列の変換テーブルの両方がここから自動生成される）。
 *     X(enum値の名前, JSON側のkey文字列)
 */
#define BOSS_TYPE_LIST(X) \
    X(enGorilla, "Gorilla") \
    X(enTurtle,  "Turtle")

/* ボスの種類 */
enum class BossType
{
    enNone = 0xFFFFFFFF, //!< 何もなし
#define BOSS_TYPE_X(name, key) name,
    BOSS_TYPE_LIST(BOSS_TYPE_X)
#undef BOSS_TYPE_X
    enBossType_Max
};

/** ボスのアニメーション */
enum BossAnimID {
    enNone = -1, //!< 何もなし
    enAnimIdle = 0,      //!< 待機
    enAnimRun,           //!< 走る
    enAnimJump,          //!< ジャンプ（ヒットスタンプ）
    enAnimAttack,        //!< 通常攻撃
    enAnimHit,           //!< ダメージリアクション
    enAnimDeath,         //!< 死亡
    enAnimSpin,          //!< 回転攻撃
    enAnimClicked,       //!< 岩投げ・レーザーの発射モーション
    enAnimAntic,         //!< レーザーの予備動作
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
    /** モデルに必要なデータ */
    std::string characterKey_ = "Turtle"; //!< キャラクター識別キー（CharacterMaster.json等のkeyと一致。ボスの識別はこれで行う）
    std::string modelPath_ = ""; //!< モデルファイルパス
    AnimSetting anims[enAnimNum]; //!< アニメーション設定一覧
    EnModelUpAxis modelAxis_ = EnModelUpAxis::enModelUpAxisZ; //!< モデルの上方向
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