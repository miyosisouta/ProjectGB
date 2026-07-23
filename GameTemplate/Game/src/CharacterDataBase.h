#pragma once

/** キャラクターパラメータ用の構造体 */
struct CharacterStatusDef
{
    uint32_t key = 0; //!< 検索用のCRC32ハッシュキー

    uint16_t hp = 0; //!< HP
    uint8_t attack = 0; //!< 攻撃力
    float speed = 0.0f; //!< 移動速度
};

/** キャラクター1体分の設定情報をまとめる構造体 */
struct PlayerParam
{
    NormalAttackType nAttack = NormalAttackType::enNone; //!< 装備中の通常攻撃
    AbilityType      ability = AbilityType::enNone; //!< 装備中の特殊能力
    UtilityType      utility = UtilityType::enNone; //!< 装備中の汎用スキル
};


/** キャラクター用データベース */
class CharacterDataBase
{
private:
    PlayerParam playerData_; //!< プレイヤー用データ保管庫

    BossParam bossParam_; //!< ボスに必要なパラメータ


private:
    /** コンストラクタ */
    CharacterDataBase();
    /** デストラクタ */
    ~CharacterDataBase();


public:
    /* ============================================================================ */
    /* プレイヤーデータ */
    /* ============================================================================ */

    /** プレイヤーパラメータの設定 */
    inline void SetPlayerParam(const PlayerParam& param) { playerData_ = param; }
    /** プレイヤーパラメータの取得 */
    inline const PlayerParam& GetPlayerParam() const { return playerData_; }

    // 特定のスキルだけを書き換える用
    /** 通常攻撃の設定 */
    inline void SetPlayerNormalAttack(NormalAttackType type) { playerData_.nAttack = type; }
    /** 特殊能力の設定 */
    inline void SetPlayerAbility(AbilityType type) { playerData_.ability = type; }
    /** 汎用スキルの設定 */
    inline void SetPlayerUtility(UtilityType type) { playerData_.utility = type; }


    /* ============================================================================ */
    /* ボスデータ */
    /* ============================================================================ */

    /** ボスのパラメータの設定 */
    inline void SetGameSettingParam(const BossParam& param) { bossParam_ = param; }
    /** ボスのパラメータの取得 */
    inline const BossParam& GetGameParam() const { return bossParam_; }

    /** ステージタイプの設定 */
    inline void SetStageType(BossType type) { bossParam_.stageType_ = type; }
    /** ステージタイプの取得 */
    inline const BossType& GetStageType() const{ return bossParam_.stageType_; }
    /** ゲームモードの設定 */
    inline void SetGameModeType(GameModeType mode) { bossParam_.mode_ = mode; }
    /** ゲームモードの取得 */
    inline const GameModeType& GetGameModeType() const { return bossParam_.mode_; }


/*=============================================*/
/* ここから下はシングルトン */
/*=============================================*/

private:
    /** 自身のインスタンス */
    static CharacterDataBase* instance_;


public:
    /**
     * インスタンスを作る
     */
    static void CreateInstance()
    {
        if (!instance_) instance_ = new CharacterDataBase();
    }


    /**
     * インスタンスを取得
     */
    static CharacterDataBase& Get()
    {
        return *instance_;
    }


    /**
     * インスタンスを破棄
     */
    static void Finalize()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};