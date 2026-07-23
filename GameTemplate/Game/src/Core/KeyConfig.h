#pragma once
#include <array>



/** ゲーム中のアクション */
enum EnGameAction
{
    /** 移動 */
    enActionMoveUp,
    enActionMoveDown,
    enActionMoveLeft,
    enActionMoveRight,

    /** 戦闘 */
    enActionNormalSkill,    //!< 通常スキル
    enActionSpecialSkill,   //!< 特殊スキル
    enActionUtilitySkill,   //!< 汎用スキル
    enActionDash,           //!< ダッシュ

    /** UI */
    enActionInteract,   //!< 決定ボタン
    enActionCancel,     //!< キャンセル
    enActionOpenMenu,   //!< メニューを開く

    /** その他 */
    enGameActionNum     //!< 設定するボタンの最大数
};


class KeyConfig
{
public:
    /** キーのセッティング用構造体 */
    struct KeySetting
    {
        int   button        = 0;     //!< 割り当てボタン
        bool  isHoldType    = false; //!< 長押し判定を行うか

        float holdThreshold = 0.5f;  //!< 長押し確定までの時間
        float pressTime     = 0.0f;  //!< 押し続けている時間
        float releaseTime   = 0.0f;  //!< 離してからの時間
        bool  wasPressed    = false; //!< 前フレームで押されていたか
        bool  holdFired     = false; //!< 長押し確定済みか
        bool  holdStart     = false; //!< 長押しが確定した瞬間か
        bool  isHolding     = false; //!< 長押し継続中か
        bool  shortRelease  = false; //!< 長押し確定前に離した瞬間か
    };

private:
    std::array<KeySetting, enGameActionNum> keyMap_; //!< アクションごとのボタン割り当て用マップ


private:
    /** コンストラクタ */
    KeyConfig();
    /** デストラクタ */
    ~KeyConfig();


public:
    /** デフォルト設定に戻す関数 */
    void ResetToDefault();

    /** 入力の更新 */
    void UpdateHoldType(KeySetting& set);

    /** 押した瞬間か */
    bool IsTrigger(EnGameAction action) const;

    /** 押し続けている */
    bool IsPress(EnGameAction action) const;

    /** 長押し開始の瞬間か */
    bool IsShortRelease(EnGameAction action) const;

    /** 長押し継続中か */
    bool IsHolding(EnGameAction action) const;

    /** キーバインドの取得 */
    int GetBindButton(EnGameAction action) const;
    /** キーバインドの設定 */
    void SetKeyBind(EnGameAction action, int bindButton);

    /** 更新 */
    void Update();


    /*========================================*/
    /* シングルトンアクセス */
    /*========================================*/

private:
    static KeyConfig* instance_; //!< インスタンス

public:
    /** インスタンスを作る */
    static void CreateInstance()
    {
        if (instance_ == nullptr)
        {
            instance_ = new KeyConfig();
        }
    }

    /** インスタンスを取得 */
    static KeyConfig& Get()
    {
        return *instance_;
    }

    /** インスタンスを破棄 */
    static void Finalize()
    {
        if (instance_ != nullptr)
        {
            delete instance_;
            instance_ = nullptr;
        }
    }
};
