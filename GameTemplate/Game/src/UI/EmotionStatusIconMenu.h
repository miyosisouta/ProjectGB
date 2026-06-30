/**
 * EmotionStatusIconMenu.h
 */
#pragma once
#include "Menu.h"

/** HP bar above status icon (attack/speed buff-debuff display) */
class EmotionStatusIconMenu : public MenuBase
{
public:
    void Update() override;
    void InitializeLogic() override;
};
