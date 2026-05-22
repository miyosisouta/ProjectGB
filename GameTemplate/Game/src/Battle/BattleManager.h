/**
 * BattleManager.h
 * インゲーム管理
 *
 * インゲームのゲームオブジェクトを統括する。
 * クラス間の情報伝達もこのクラスを介して行う。
 *
 * ============================================================
 * 【グループビット制御】
 *
 *   Player, Boss, UI, Stage, Camera の5グループを
 *   updateMask_ / drawMask_ のビット演算で個別に ON/OFF できる。
 *
 *   - Player / Boss : Character::SetUpdate(bool) で Update 制御
 *                     Render は GameObjectManager 経由で常に有効
 *   - Camera        : CameraSteering::SetUpdate(bool) で Update 制御
 *   - Stage         : StageManagerObject::SetUpdate(bool) で Update 制御
 *   - UI            : UIManager の updateMask / drawMask で制御
 *
 *   呼び出し例:
 *     // ポーズ: UIのみ更新、全部描画
 *     BattleManager::Get().SetGroupMask(UpdateGroup::UI, UpdateGroup::All);
 *
 *     // 全解除
 *     BattleManager::Get().SetGroupMask(UpdateGroup::All);
 *
 * ============================================================
 */

#pragma once
#include "src/Camera/CameraController.h"
#include "src/Camera/CameraSteering.h"
#include "src/Util/GameTimer.h"
#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"
#include "src/UI/UIManager.h"


class PlayerController;
class BossSpawner;
class StageManagerObject;


class BattleManager
{
private:
	enum class GameState
	{
		Entry,          // 開始前の演出
		GameStart,      // ゲームスタート演出
		Playing,        // メインのゲームプレイ中
		ResultClear,    // スコア表示・リザルト演出
		ResultOver,     // スコア表示・リザルト演出
		Shutdown        // 終了処理
	};

	/** カメラオプション */
	struct CameraOption
	{
		bool  invert      = false;    //!< 操作反転
		float sensitivity = 1.0f;    //!< 感度
		float distance    = 50.0f;   //!< 距離
		float fovDeg      = 60.0f;   //!< 視野角
	};


private:
	// --- 各ゲームオブジェクト ---
	Player*             player_           = nullptr;
	PlayerController*   playerController_ = nullptr;
	BossSpawner*        boss_             = nullptr;
	StageManagerObject* stage_            = nullptr;
	GameTimer           gameTimer_;
	SkyCube*            skyCube_          = nullptr;

	// --- UI 管理 ---
	UIManager           uiManager_;

	// --- カメラ ---
	std::unique_ptr<CameraSteering>    cameraSteering_           = nullptr;
	std::unique_ptr<TaskSchedulerSystem> cutSceneScheduler_      = nullptr;
	std::unique_ptr<TaskSchedulerSystem> bossMoveStartScheduler_ = nullptr;
	RefCameraController                gameCameraController_     = nullptr;
	RefCameraController                bossEntryCameraController_ = nullptr;

private:
	GameState gameState_        = GameState::Entry;
	uint32_t  updateMask_       = UpdateGroup::All;  //!< 更新グループマスク
	uint32_t  drawMask_         = UpdateGroup::All;  //!< 描画グループマスク
	bool      isPlayingEntryBoss_ = true;
	bool      isPlayingResult_    = false;



	/*===================================================*/
	/* セッター・ゲッター                                */
	/*===================================================*/
public:
	/**
	 * カメラオプションを取得
	 * 設定変更前にこれで現在値を取得してください
	 */
	CameraOption GetCameraOption() const
	{
		CameraOption option;
		option.invert       = cameraSteering_->GetInvert();
		option.sensitivity  = cameraSteering_->GetSensitivity();
		option.distance     = cameraSteering_->GetDistance();
		option.fovDeg       = gameCameraController_->As<GameCamera>()->GetFovDeg();
		return option;
	}

	/** カメラオプションを設定 */
	void SetCameraOption(const CameraOption& option)
	{
		cameraSteering_->SetInvert(option.invert);
		cameraSteering_->SetSensitivity(option.sensitivity);
		cameraSteering_->SetDistance(option.distance);
		gameCameraController_->As<GameCamera>()->SetFovDeg(option.fovDeg);
	}

	/**
	 * 更新・描画のグループマスクを設定する。
	 *
	 * @param updateMask  更新するグループのビットマスク (UpdateGroup::*)
	 * @param drawMask    描画するグループのビットマスク (UpdateGroup::*)
	 *                    省略時は updateMask と同じ値が適用される
	 *
	 * 例: ポーズ
	 *   SetGroupMask(UpdateGroup::UI, UpdateGroup::All);
	 *       → UI だけ更新し、全グループは描画し続ける
	 *
	 * 例: 全解除
	 *   SetGroupMask(UpdateGroup::All);
	 */
	void SetGroupMask(uint32_t updateMask, uint32_t drawMask = UpdateGroup::All);

	/** ポーズ状態を設定する（ゲームタイマーの停止・再開） */
	void SetPouse(const bool flg)
	{
		if (flg) gameTimer_.Pause();
		else     gameTimer_.Resume();
	}

	/* ゲームタイマーを取得 */
	GameTimer* GetGameTimer() { return &gameTimer_; }

	/* プレイヤーのHP割合を取得 */
	float GetPlayerHPRate()
	{
		auto* status = player_->GetStatus()->As<ActorStatus>();
		return static_cast<float>(status->GetHP()) / static_cast<float>(status->GetMaxHP());
	}

	/** UIManager を取得（細かい UI 制御が必要な場合に使用） */
	UIManager& GetUIManager() { return uiManager_; }


private:
	BattleManager();
	~BattleManager();


public:
	void Update();
	void Render(RenderContext& rc);


public:
	/** カットシーン中（Playing 以外）か */
	bool IsCutScene()     const { return gameState_ != GameState::Playing; }
	/** プレイ中か */
	bool IsPlayingScene() const { return gameState_ == GameState::Playing; }
	/** ゲーム演出が完全に終了したか */
	bool IsFinishedGame() const { return gameState_ == GameState::Shutdown; }


private:
	/** updateMask_ / drawMask_ を各オブジェクトに反映する */
	void ApplyGroupMasks();

	bool UpdateEntryBoss();
	bool UpdateResultClear();
	bool UpdateResultOver();

	void SetupEntryBossCutScene();
	void SetupStartCutScene();
	void SetupClearCutScene();
	void SetupOverCutScene();

#ifdef K2_DEBUG
	/**
	 * デバッグ用グループトグル入力
	 *
	 * F1 : Player  更新 ON/OFF
	 * F3 : UI      更新 ON/OFF
	 * F4 : Stage   更新 ON/OFF
	 * F5 : Boss    更新 ON/OFF
	 * F6 : Effect  更新 ON/OFF（Effekseer を一時停止）
	 * F7 : Sound   更新 ON/OFF
	 * F8 : UI      描画 ON/OFF
	 */
	void UpdateDebugGroupInput();
#endif


	/*===================================*/
	/* シングルトン関連                  */
	/*===================================*/
private:
	static BattleManager* myInstance_;

public:
	static void CreateInstance()
	{
		if (!myInstance_) myInstance_ = new BattleManager();
	}

	static BattleManager& Get()
	{
		if (myInstance_) return *myInstance_;
	}

	static void DestroyInstance()
	{
		if (myInstance_) {
			delete myInstance_;
			myInstance_ = nullptr;
		}
	}
};
