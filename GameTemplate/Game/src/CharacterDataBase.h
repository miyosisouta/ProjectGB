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

    bool isTutorialEnabled_ = true; //!< チュートリアルを行うか。選択UI側から設定される想定（デフォルトtrue＝行う）


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

    /**
     * ステージ（ボス）の識別キーの設定。
     * CharacterMaster.json / AnimationMaster.json / CharacterStatusData.json 等の"key"と一致させる。
     * ボス選択画面（TitleScene）から、選ばれたボスのkeyをそのまま渡してもらう想定。
     */
    inline void SetStageKey(const std::string& key) { bossParam_.characterKey_ = key; }
    /** ステージ（ボス）の識別キーの取得 */
    inline const std::string& GetStageKey() const { return bossParam_.characterKey_; }


    /* ============================================================================ */
    /* チュートリアル */
    /* ============================================================================ */

    /**
     * チュートリアルを行うかどうかを設定する（true=行う／false=飛ばして最初から全行動解禁でボス登場演出から始める）
     * TODO: チュートリアルを行うか選ぶUI画面が実装されたら、その決定処理からここを呼んでください。
     *       呼び出し例は TitleScene.cpp の SetStageKey 呼び出し箇所を参照。
     *       未設定の場合はデフォルトtrue（チュートリアルを行う）のままです。
     */
    inline void SetTutorialEnabled(bool flg) { isTutorialEnabled_ = flg; }
    /** チュートリアルを行うかどうかを取得 */
    inline bool IsTutorialEnabled() const { return isTutorialEnabled_; }


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