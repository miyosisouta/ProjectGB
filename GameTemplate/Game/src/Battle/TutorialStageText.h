/**
 * TutorialStageText.h
 * チュートリアルの現在の段階名を画面右側に仮表示するデバッグ表示。
 *
 * 表示位置・スタイルの管理だけを担い、「何を表示すべきか」の判断はTutorialManager側が行う。
 */
#pragma once


class TutorialStageText
{
private:
	FontRender fontRender_;

public:
	/** コンストラクタ：表示位置・大きさを設定する */
	TutorialStageText();

	/** 表示するテキストを差し替える */
	void SetText(const wchar_t* text);

	/** 描画処理 */
	void Render(RenderContext& rc);
};
