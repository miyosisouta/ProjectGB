#pragma once

// キャラクター1体分の設定情報をまとめる構造体
struct CharacterParam 
{
    NormalAttackType nAttack = NormalAttackType::enNone;
    AbilityType      ability = AbilityType::enNone;
    UtilityType      utility = UtilityType::enNone;
};

class CharacterDataBase
{
private:
    CharacterParam playerData_; // プレイヤー用データ保管庫
    CharacterParam bossData_;   // ボス用データ保管庫


public:
    /* プレイヤー用ゲッター・セッター */
    void SetPlayerParam(const CharacterParam& param) { playerData_ = param; }
    const CharacterParam& GetPlayerParam() const { return playerData_; }

    // 特定のスキルだけを書き換える用
    void SetPlayerNormalAttack(NormalAttackType type) { playerData_.nAttack = type; }
    void SetPlayerAbility(AbilityType type) { playerData_.ability = type; }
    void SetPlayerUtility(UtilityType type) { playerData_.utility = type; }

    /* ボス用ゲッター・セッター */
    void SetBossParam(const CharacterParam& param) { bossData_ = param; }
    const CharacterParam& GetBossParam() const { return bossData_; }


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