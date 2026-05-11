/**
 * UIManager.h
 * インゲームUI管理
 *
 * BattleManager 配下の UI レイアウトを一元管理し、
 * ビット演算によるグループ単位の Update / Render 制御を提供する。
 *
 * ============================================================
 * 【使用例】
 *   // UIのみ停止（ポーズ解除時など）
 *   uiManager_.SetUpdateMask(UIManager::None);
 *
 *   // ミッションUIだけ非表示
 *   uiManager_.SetDrawMask(UIManager::Timer | UIManager::CutScene);
 */

#pragma once
#include "Layout.h"

class TimerMenu;


class UIManager
{
public:
	// ============================================================
	// UIグループのビットフラグ
	// ============================================================
	enum : uint32_t
	{
		Mission  = 1 << 0,  //!< ミッション表示 (MissionMenu)
		Timer    = 1 << 1,  //!< タイマー表示  (TimerMenu)
		CutScene = 1 << 2,  //!< カットシーン系UI (BossEntry / Start / Clear / Over)
		Screen   = 1 << 3,  //!< UIScreenManager 管理下のスクリーン
		All  = Mission | Timer | CutScene | Screen,
		None = 0,
	};

private:
	Layout* missionLayout_  = nullptr;  //!< ミッション表示（常駐）
	Layout* timerLayout_    = nullptr;  //!< タイマー表示（Playing 以降）
	Layout* cutSceneLayout_ = nullptr;  //!< カットシーン / ゲーム開始 / リザルトUI

	uint32_t updateMask_ = All;
	uint32_t drawMask_   = All;


public:
	UIManager()  = default;
	~UIManager();

	UIManager(const UIManager&)            = delete;
	UIManager& operator=(const UIManager&) = delete;


	// ============================================================
	// セットアップ
	// ============================================================

	/** ミッション UI を生成する */
	void InitMission();

	/** タイマー UI を生成する */
	void InitTimer();

	/**
	 * カットシーン UI を生成する。
	 * 既存の CutScene レイアウトは自動解放される。
	 * テンプレート引数に MenuBase の派生クラスを指定すること。
	 */
	template <typename TMenu>
	void SetupCutScene(const std::string& path)
	{
		ReleaseCutSceneLayout();
		cutSceneLayout_ = new Layout();
		cutSceneLayout_->Initialize<TMenu>(path);
	}

	/** カットシーン UI を解放する */
	void ReleaseCutSceneLayout();


	// ============================================================
	// アクセサ
	// ============================================================

	/** カットシーン Menu を取得 */
	MenuBase* GetCutSceneMenu() const
	{
		return cutSceneLayout_ ? cutSceneLayout_->GetMenu() : nullptr;
	}

	/** Timer Menu を取得 */
	TimerMenu* GetTimerMenu() const;


	// ============================================================
	// マスク制御
	// ============================================================

	/** 更新グループマスクを設定する */
	void SetUpdateMask(uint32_t mask) { updateMask_ = mask; }

	/** 描画グループマスクを設定する */
	void SetDrawMask(uint32_t mask)   { drawMask_   = mask; }

	uint32_t GetUpdateMask() const { return updateMask_; }
	uint32_t GetDrawMask()   const { return drawMask_;   }


	// ============================================================
	// Update / Render
	// ============================================================

	void Update();
	void Render(RenderContext& rc);
};
