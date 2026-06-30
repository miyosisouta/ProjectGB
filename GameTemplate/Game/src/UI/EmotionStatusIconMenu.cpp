/**
 * EmotionStatusIconMenu.cpp
 */
#include "stdafx.h"
#include "EmotionStatusIconMenu.h"
#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"
#include "src/Emotion/EmotionSystem.h"

namespace
{
    const Vector4 COLOR_BUFF   = { 0.0f, 0.8f, 1.0f, 1.0f }; // Buff: blue
    const Vector4 COLOR_DEBUFF = { 1.0f, 0.0f, 0.0f, 1.0f }; // Debuff: red
    const Vector4 COLOR_NORMAL = Vector4::White;
}


void EmotionStatusIconMenu::Update()
{
    int level = 0;
    if (auto* player = FindGO<Player>("player"))
    {
        if (auto* st = player->GetStatus()->As<PlayerStatus>())
        {
            level = static_cast<int>(st->GetEmotionSystem().GetCurrentLevel());
        }
    }

    Vector4 color = COLOR_NORMAL;
    if (level > 0) color = COLOR_BUFF;
    else if (level < 0) color = COLOR_DEBUFF;

    auto applyColor = [&](const char* back, const char* border, const char* icon)
        {
            if (auto* p = GetUI<UIIcon>(Hash32(back)))   p->color = color;
            if (auto* p = GetUI<UIIcon>(Hash32(border))) p->color = color;
            if (auto* p = GetUI<UIIcon>(Hash32(icon)))   p->color = color;
        };

    // Speed系だけ表示/非表示を切り替えるヘルパー
    auto applyVisible = [&](const char* back, const char* border, const char* icon, bool visible)
        {
            if (auto* p = GetUI<UIIcon>(Hash32(back)))   p->isDraw = visible;
            if (auto* p = GetUI<UIIcon>(Hash32(border))) p->isDraw = visible;
            if (auto* p = GetUI<UIIcon>(Hash32(icon)))   p->isDraw = visible;
        };

    applyColor("Attack_Icon_back", "Attack_Icon_border", "Attack_Icon_image");
    applyColor("Speed_Icon_back", "Speed_Icon_border", "Speed_Icon_image");

    // バフ中（level > 0）は速度バフが存在しないので Speed アイコンを非表示にする
    bool speedIconVisible = (level <= 0);
    applyVisible("Speed_Icon_back", "Speed_Icon_border", "Speed_Icon_image", speedIconVisible);

    int absLevel = level < 0 ? -level : level;
    wchar_t numText[4] = L"";
    if (absLevel > 0)
    {
        swprintf_s(numText, L"%d", absLevel);
    }

    if (auto* t = GetUI<UIText>(Hash32("Attack_Level_text"))) t->SetText(numText);

    if (auto* t = GetUI<UIText>(Hash32("Speed_Level_text")))
    {
        t->isDraw = speedIconVisible;          // テキストも合わせて非表示
        t->SetText(speedIconVisible ? numText : L"");
    }

    MenuBase::Update();
}

void EmotionStatusIconMenu::InitializeLogic()
{
}
