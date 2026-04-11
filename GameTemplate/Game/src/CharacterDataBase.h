#pragma once

/** キャラクターパラメータ用の構造体 */
struct CharacterStatusDef
{
    uint32_t key = 0; // 検索用のCRC32ハッシュキー

    uint16_t hp = 0;
    uint8_t attack = 0;
    float speed = 0.0f;
};

// キャラクター1体分の設定情報をまとめる構造体
struct PlayerParam
{
    NormalAttackType nAttack = NormalAttackType::enNone;
    AbilityType      ability = AbilityType::enNone;
    UtilityType      utility = UtilityType::enNone;
};


/* キャラクター用データベース */
class CharacterDataBase
{
private:
    PlayerParam playerData_; //!< プレイヤー用データ保管庫

    BossParam bossParam_; //!< ボスに必要なパラメータ


public:
    /* ============================================================================ */
    /* プレイヤーデータ */
    /* ============================================================================ */

    /* プレイヤー用ゲッター・セッター */
    void SetPlayerParam(const PlayerParam& param) { playerData_ = param; }
    const PlayerParam& GetPlayerParam() const { return playerData_; }

    // 特定のスキルだけを書き換える用
    void SetPlayerNormalAttack(NormalAttackType type) { playerData_.nAttack = type; }
    void SetPlayerAbility(AbilityType type) { playerData_.ability = type; }
    void SetPlayerUtility(UtilityType type) { playerData_.utility = type; }


    /* ============================================================================ */
    /* ボスデータ */
    /* ============================================================================ */

    /* ボスのパラメータ */
    void SetGameSettingParam(const BossParam& param) { bossParam_ = param; }   //!< 設定
    const BossParam& GetGameParam() const { return bossParam_; }           //!< 取得

    /* ステージタイプ */
    void SetStageType(BossType type) { bossParam_.stageType_ = type; }    //!< 設定
    const BossType& GetStageType() const{ return bossParam_.stageType_; } //!< 取得
    /* ゲームモード */
    void SetGameModeType(GameModeType mode) { bossParam_.mode_ = mode; }      //!< 設定
    const GameModeType& GetGameModeType() const { return bossParam_.mode_; }  //!< 取得


private:
    CharacterDataBase();
    ~CharacterDataBase(); 


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
    static void DestroyInstance()
    {
        if (instance_) {
            delete instance_;
            instance_ = nullptr;
        }
    }
};