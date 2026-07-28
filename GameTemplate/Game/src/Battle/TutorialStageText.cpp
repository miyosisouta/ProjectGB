#include "stdafx.h"
#include "TutorialStageText.h"


namespace
{
	// 画面右側の表示位置・大きさ（仮値。実際の見え方に合わせて調整して良い）
	constexpr float kTextPosX = -800.0f;
	constexpr float kTextPosY = 200.0f;
	constexpr float kTextScale = 1.2f;
}


TutorialStageText::TutorialStageText()
{
	fontRender_.SetPosition(kTextPosX, kTextPosY, 0.0f);
	fontRender_.SetScale(kTextScale);
}


void TutorialStageText::SetText(const wchar_t* text)
{
	fontRender_.SetText(text);
}


void TutorialStageText::Render(RenderContext& rc)
{
	fontRender_.Draw(rc);
}
