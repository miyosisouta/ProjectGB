#include "stdafx.h"
#include "KeyConfig.h"



namespace
{
    constexpr uint8_t PAD_NUM = 0;
    const float CHATTERING_MARGIN = 0.05f;
}

KeyConfig* KeyConfig::instance_ = nullptr; //初期化


void KeyConfig::ResetToDefault()
{
    KeyConfig::KeySetting setting;

    /****** インゲーム ****/
    /** うえ */
    setting.button = nsK2EngineLow::enButtonUp;
    setting.isHoldType = false;
    keyMap_[enActionMoveUp] = setting;

    /** した */
    setting.button = nsK2EngineLow::enButtonDown;
    setting.isHoldType = false;
    keyMap_[enActionMoveDown] = setting;

    /** 左 */
    setting.button = nsK2EngineLow::enButtonLeft;
    setting.isHoldType = false;
    keyMap_[enActionMoveLeft] = setting;

    /** 右 */
    setting.button = nsK2EngineLow::enButtonRight;
    setting.isHoldType = false;
    keyMap_[enActionMoveRight] = setting;

    /** 通常スキル */
    setting.button = nsK2EngineLow::enButtonB;
    setting.isHoldType = false;
    keyMap_[enActionNormalSkill] = setting;

    /** 特殊スキル */
    setting.button = nsK2EngineLow::enButtonY;
    setting.isHoldType = false;
    keyMap_[enActionSpecialSkill] = setting;

    /** 汎用スキル */
    setting.button = nsK2EngineLow::enButtonA;
    setting.isHoldType = false;
    keyMap_[enActionUtilitySkill] = setting;

    /** ダッシュ */
    setting.button = nsK2EngineLow::enButtonA;
    setting.holdThreshold = 0.25f;
    setting.isHoldType = true;
    keyMap_[enActionDash] = setting;


    /****** UI ****/
    /** 決定 */
    setting.button = nsK2EngineLow::enButtonA;
    setting.isHoldType = false;
    keyMap_[enActionInteract] = setting;

    /** キャンセル */
    setting.button = nsK2EngineLow::enButtonB;
    setting.isHoldType = false;
    keyMap_[enActionCancel] = setting;

    /** メニューを開く */
    setting.button = nsK2EngineLow::enButtonStart;
    setting.isHoldType = false;
    keyMap_[enActionOpenMenu] = setting;
}

void KeyConfig::UpdateHoldType(KeySetting& set)
{
    const auto btn = static_cast<nsK2EngineLow::EnButton>(set.button);
    const bool isPressed = nsK2EngineLow::g_pad[PAD_NUM]->IsPress(btn);
    const float dt = g_gameTime->GetFrameDeltaTime();

    // 1フレーム限りのフラグをリセット
    set.holdStart = false;
    set.shortRelease = false;


    if (isPressed)
    {
        if (!set.wasPressed)
        {
            // 押した瞬間
            // 猶予時間内に再度押され、かつすでに長押し状態だった場合はタイマーをリセットしない
            if (!(set.holdFired && set.releaseTime < CHATTERING_MARGIN))
            {
                // 純粋な新規の押し始め
                set.pressTime = 0.0f;
                set.holdFired = false;
            }
        }

        set.releaseTime = 0.0f; // 押されている間は離しタイマーをリセット
        set.pressTime += dt;

        if (!set.holdFired && set.pressTime >= set.holdThreshold)
        {
            // 長押し確定の瞬間
            set.holdFired = true;
            set.holdStart = true;
        }
        set.isHolding = set.holdFired;
    }
    else
    {
        // 離している間
        set.releaseTime += dt;

        if (set.wasPressed)
        {
            // 離した瞬間
            if (!set.holdFired)
            {
                // 長押し確定前に離されたので「回避」判定
                set.shortRelease = true;
            }
        }
        // 猶予時間を完全に超えて離されていたら、ホールド状態を確実に落とす
        if (set.releaseTime >= CHATTERING_MARGIN)
        {
            set.isHolding = false;
            set.holdFired = false;
            set.pressTime = 0.0f;
        }
        else if (set.holdFired)
        {
            // 猶予時間内ならプレイヤーが意図せず指を浮かせただけと判断しホールドを維持
            set.isHolding = true;
        }
    }
    set.wasPressed = isPressed;
}

/*========================================*/
/* 入力判定                               */
/*========================================*/

bool KeyConfig::IsTrigger(EnGameAction action) const    
{
    const auto btn = static_cast<nsK2EngineLow::EnButton>(keyMap_[action].button);
    return g_pad[PAD_NUM]->IsTrigger(btn);
}

bool KeyConfig::IsPress(EnGameAction action) const
{
    const auto btn = static_cast<nsK2EngineLow::EnButton>(keyMap_[action].button);
    return g_pad[PAD_NUM]->IsPress(btn);
}

bool KeyConfig::IsShortRelease(EnGameAction action) const
{
    return keyMap_[action].shortRelease;
}

bool KeyConfig::IsHolding(EnGameAction action) const
{
    return keyMap_[action].isHolding;
}


int KeyConfig::GetBindButton(EnGameAction action) const
{
    return keyMap_[action].button;
}

void KeyConfig::SetKeyBind(EnGameAction action, int bindButton)
{
    keyMap_[action].button = bindButton;
}


// =========================================================
// コンストラクタ
// =========================================================

KeyConfig::KeyConfig()
{
    ResetToDefault();
}

KeyConfig::~KeyConfig()
{
}

void KeyConfig::Update()
{
    for (auto& setting : keyMap_)
    {
        if (setting.isHoldType)
        {
            UpdateHoldType(setting);
        }
    }
}
