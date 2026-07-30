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
#include "src/Actor/ActorStatus.h"
#include "src/Actor/Player.h"
#include "src/Camera/CameraController.h"
#include "src/Camera/CameraSteering.h"
#include "src/UI/UIManager.h"
#include "src/Util/GameTimer.h"
#include "src/Util/DamageNotify.h"
#include "src/Util/Curve.h"
#include "src/Battle/BossEmotionPhaseManager.h"
#include "src/Battle/TutorialManager.h"
#include "src/Emotion/EmotionEffectObserver.h"
#include "src/Core/ParameterManager.h"


class PlayerController;
class BossSpawner;
class BossCharacter;
class StageManagerObject;
class BossPhaseCutSceneMenu;


class BattleManager
{
private:
	/** ゲームの進行状態 */
	enum class GameState
	{
		Tutorial,          //!< チュートリアル（ボス登場演出より前）
		TutorialLoading,   //!< チュートリアル終了〜ボス登場演出の間のLoading演出
		Entry,             //!< 開始前の演出
		GameStart,      //!< ゲームスタート演出
		Playing,        //!< メインのゲームプレイ中
		ResultClear,    //!< スコア表示・リザルト演出
		ResultOver,     //!< スコア表示・リザルト演出
		Shutdown        //!< 終了処理
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
	Player*             player_           = nullptr; //!< プレイヤー本体
	PlayerController*   playerController_ = nullptr; //!< プレイヤーの操作コントローラー
	BossSpawner*        boss_             = nullptr; //!< ボス生成・管理
	std::string         realStageKey_;                //!< プレイヤーが実際に選んだボスのkey（チュートリアルは常にGorillaで行うため、本番切り替え用に覚えておく）
	StageManagerObject* stage_            = nullptr; //!< ステージ管理
	GameTimer           gameTimer_; //!< ゲーム内タイマー
	SkyCube*            skyCube_          = nullptr; //!< 天球オブジェクト

	// --- UI 管理 ---
	UIManager           uiManager_;

	// --- カメラ ---
	std::unique_ptr<CameraSteering>    cameraSteering_           = nullptr; //!< カメラの操作入力を管理
	std::unique_ptr<TaskSchedulerSystem> cutSceneScheduler_      = nullptr; //!< カットシーン演出用タイマー
	RefCameraController                gameCameraController_     = nullptr; //!< 通常プレイ用カメラ
	RefCameraController                bossEntryCameraController_ = nullptr; //!< ボス登場演出用カメラ

	// --- ダメージ ---
	DamageNotifyQueue damageNotifyQueue_; //!< ダメージ通知管理

	// --- ゲームフェーズ管理 ---
	// ボスのHP閾値監視と、感情システムへの強制変更を担う（Mediatorパターン）
	BossEmotionPhaseManager bossEmotionPhaseManager_;
	EmotionEffectObserver   emotionEffectObserver_;                  //!< 感情レベル変化時にエフェクトを再生するオブザーバー
	bool bossEmotionPhaseManagerInitialized_ = false; //!< Start()完了後に1回だけ Init() するためのフラグ
	bool tutorialInitialized_ = false; //!< Tutorial突入時に1回だけ初期化処理を行うためのフラグ
	bool tutorialSkipKeyPrevPressed_ = false; //!< チュートリアルスキップ用Enterキーの前フレーム押下状態（立ち上がり検知用）
	float tutorialLoadingElapsed_ = 0.0f; //!< TutorialLoading突入からの経過秒数

	// --- ボスHP閾値カットシーン（HP50%/25%でボスへズームアウトする演出） ---
	bool bossPhase50CutsceneTriggered_ = false; //!< HP50%閾値カットシーンの発火済みフラグ（重複発火を防ぐ）
	bool bossPhase25CutsceneTriggered_ = false; //!< HP25%閾値カットシーンの発火済みフラグ
	bool isBossPhaseCutsceneActive_    = false; //!< 演出中〜プレイヤーが操作可能に戻るまでtrue（InGameSceneがインゲームHUDの描画/更新可否を判断するのに使う）

	// ボスHP閾値カットシーンのカメライージング用（0=通常カメラ位置, 1=ボス正面位置）
	FloatCurve bossPhaseCutsceneCameraBlendCurve_;      //!< イージング進行度（0→1で正面へ、1→0で復帰）
	CameraData bossPhaseCutsceneCameraStartData_;       //!< イージング開始時点の通常カメラ（＝復帰先でもある）
	CameraData bossPhaseCutsceneCameraTargetData_;      //!< イージング先のボス正面カメラ

	int bossPhaseCutsceneJumpsRemaining_ = 0; //!< HP50%演出：残りジャンプ回数。ジャンプ（着地）が終わるたびに減らし、0になったらIdleに戻す

	// HP25%演出：ボスの前後傾き（X軸）・左右揺れ（Y軸）イージング用
	FloatCurve bossPhaseCutscenePitchCurve_;             //!< 前後傾き角度(度)のイージング進行
	FloatCurve bossPhaseCutsceneYawCurve_;               //!< 左右揺れ角度(度)のイージング進行
	Quaternion bossPhaseCutscenePitchBaseRotation_;      //!< イージングの基準となるボスの向き（カメラ到着時点のもの）
	float      bossPhaseCutscenePitchCurrentDeg_ = 0.0f; //!< 直近の前後傾き角度(度)。カーブが停止中も保持し続ける
	float      bossPhaseCutsceneYawCurrentDeg_   = 0.0f; //!< 直近の左右揺れ角度(度)。カーブが停止中も保持し続ける


private:
	GameState gameState_        = GameState::Tutorial;
	uint32_t  updateMask_       = UpdateGroup::All;  //!< 更新グループマスク
	uint32_t  drawMask_         = UpdateGroup::All;  //!< 描画グループマスク
	bool      isPlayingEntryBoss_ = true;
	bool      isPlayingResult_    = false;
	bool      isWorldFrozen_      = false; //!< SetWorldFrozen(true)中か


private:
	/* ダメージ通知バッファ用変数 */
	static constexpr int kMaxNotify = 16;			//!< 通知の最大件数
	DamageNotify damageNotifyBuffer_[kMaxNotify];	//!< 固定長の通知バッファ
	int notifyHead_ = 0;							//!< 読み取り位置
	int notifyTail_ = 0;							//!< 書き込み位置
	int notifyCount_ = 0;							//!< 現在の通知件数


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


	public:
		/**
		 * ダメージ通知を積む
		 * CollisionHitManager のコールバックから呼ばれる
		 */
		inline void PushDamageNotify(int damage, DamageNotifyType type, bool isCritical)
		{
			damageNotifyQueue_.Push(damage, type, isCritical);
		}

		/**
		 * ダメージ通知を1件取得する
		 * 取得と同時に通知は自動的に削除される
		 * UI側はこれを呼ぶだけでよい
		 * @return 通知1件分の構造体（通知がなければ damage が -1）
		 */
		inline DamageNotify PopDamageNotify()
		{
			return damageNotifyQueue_.Pop();
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

	/** ゲームタイマーを取得 */
	inline GameTimer* GetGameTimer() { return &gameTimer_; }

	/** プレイヤーのHP割合を取得 */
	float GetPlayerHPRate()
	{
		auto* status = player_->GetStatus()->As<ActorStatus>();
		return static_cast<float>(status->GetHP()) / static_cast<float>(status->GetMaxHP());
	}

	/** UIManager を取得（細かい UI 制御が必要な場合に使用） */
	inline UIManager& GetUIManager() { return uiManager_; }

	/** カットシーン中（Playing / Tutorial 以外）か。InGameSceneはこれでインゲームHUDの描画/更新を切り替える */
	inline bool IsCutScene()     const { return gameState_ != GameState::Playing && gameState_ != GameState::Tutorial; }
	/**
	 * チュートリアル中か（チュートリアル終了後のLoading演出中を含む）。
	 * EnterキーがパッドのStartボタンと物理的に衝突するため、InGameSceneのポーズ判定を止めるのに使う。
	 * TutorialLoadingを含めないと、Enterでチュートリアルをスキップした瞬間に
	 * gameState_がTutorialLoadingへ遷移済みになり、同じEnter入力でポーズも開いてしまう
	 */
	inline bool IsTutorialScene() const { return gameState_ == GameState::Tutorial || gameState_ == GameState::TutorialLoading; }
	/** プレイ中か */
	inline bool IsPlayingScene() const { return gameState_ == GameState::Playing; }
	/** ゲーム演出が完全に終了したか */
	inline bool IsFinishedGame() const { return gameState_ == GameState::Shutdown; }

	/** ボスHP閾値カットシーン中（プレイヤーが操作可能に戻るまで）か。InGameSceneがインゲームHUDの描画/更新を止めるかどうかの判断に使う */
	inline bool IsBossPhaseCutsceneActive() const { return isBossPhaseCutsceneActive_; }


private:
	/** コンストラクタ */
	BattleManager();
	/** デストラクタ */
	~BattleManager();


public:
	/** 更新処理 */
	void Update();
	/** 描画処理 */
	void Render(RenderContext& rc);


private:
	/** updateMask_ / drawMask_ を各オブジェクトに反映する */
	void ApplyGroupMasks();

	/**
	 * GameState::Tutorial の毎フレーム更新をまとめたもの。
	 * 初回初期化・Enterキーによるスキップ入力検知・TutorialManagerへの進行判定委譲・
	 * 完了時のGameState::TutorialLoadingへの遷移セットアップを行う。
	 */
	void UpdateTutorialState();

	/**
	 * GameState::TutorialLoading の毎フレーム更新をまとめたもの。
	 * チュートリアル終了からボス登場演出までの間、Loading演出を既定秒数(TUTORIAL_LOADING_DURATION)だけ挟む。
	 * 経過後、ボス登場演出をセットアップしてGameState::Entryへ遷移する。
	 */
	void UpdateTutorialLoadingState();

	/**
	 * Tutorial / Playing の両方で毎フレーム必要な共通処理をまとめたもの。
	 * ボスの感情システム更新・ボスの見た目更新・草の押し倒し/復元・ディザリング・カメラ/スカイキューブのプレイヤー追従を行う。
	 * Playing 固有の処理（ミッション・タイマー・HP閾値カットシーン等）はここには含めない。
	 */
	void UpdateSharedGameplaySystems();

	/**====================================*/
	/** シーン演出 */
	/**====================================*/
	
	/** ボスの登場シーンの更新 */
	bool UpdateEntryBoss();
	/** クリアリザルトの更新 */
	bool UpdateResultClear();
	/** ゲームオーバーリザルトの更新 */
	bool UpdateResultOver();

	/** ゴリラの登場演出シーン */
	void SetupEntryGolliraCutScene();
	/** カメの登場演出シーン */
	void SetupEntryTurtleCutScene();

	/** ゲームスタートのカットのセットアップ */
	void SetupStartCutScene();
	/** クリアカットシーンのセットアップ */
	void SetupClearCutScene();
	/** オーバーカットシーンのセットアップ */
	void SetupOverCutScene();

	/**====================================*/
	/** ボスの特定の条件下によるカット演出 */
	/**====================================*/

	/** ボスHPが50%/25%を切った瞬間に一度だけ演出を開始する（Playing の毎フレーム呼ぶ） */
	void CheckBossPhaseCutsceneTrigger();
	/** プレイヤー・ボスを止めた上で、ボスの種類ごとのカットシーン演出を開始する。isPhase25はHP25%側の演出内容(疲れマーク等)を出し分けるためのフラグ */
	void EnterBossPhaseCutscene(const bool isPhase25);
	/** ゴリラ用: ボスHP閾値カットシーンのカメラ・演出を組み立てる */
	void SetupGorillaPhaseCutsceneCamera(BossCharacter* boss, const bool isPhase25);
	/** カメ用: ボスHP閾値カットシーンのカメラ・演出を組み立てる */
	void SetupTurtlePhaseCutsceneCamera(BossCharacter* boss, const bool isPhase25);
	/** ボス別の Setup*PhaseCutsceneCamera() から呼ぶ共通処理: カメラ切り替え＋演出タイマーの登録 */
	void StartBossPhaseCutsceneCamera(BossCharacter* boss, const bool isPhase25, const CameraData& cutsceneCameraData, const MasterBossPhaseCutSceneParameter& cutSceneParam);

	/** ボスHP閾値カットシーン中のカメライージングを1フレーム分進める（Playing中に毎フレーム呼ぶ） */
	void UpdateBossPhaseCutsceneCamera(const float deltaTime);

	/** HP25%演出：ボスの前後傾き（X軸）・左右揺れ（Y軸）イージングを1フレーム分進める（Playing中に毎フレーム呼ぶ） */
	void UpdateBossPhaseCutsceneTilt(const float deltaTime);

	/** カットシーン用UIを怒り/疲れマーク演出付きのMenuとして取得する（未セットアップ/型不一致ならnullptr） */
	BossPhaseCutSceneMenu* GetBossPhaseCutSceneMenu() const;

public:
	/**
	 * プレイヤー・ボスを操作不能にし、再生中のエフェクト・サウンド・BGMをすべて止める。
	 * カットシーンなど、プレイヤー・ボスの動きを止めたい場面であれば他からも汎用的に呼び出してよい。
	 */
	void FreezePlayerAndBoss();

	/**
	 * FreezePlayerAndBoss() で止めたプレイヤー・ボスの操作を元に戻す。
	 * ボスは必ずIdle状態から操作を再開する。
	 */
	void UnfreezePlayerAndBoss();

	/**
	 * ボスHP閾値カットシーンを終了し、プレイヤー操作とボスAIを元に戻す。
	 * ズームアウト演出（カメラワーク）を実装した際、その演出の終了タイミングで呼ぶこと。
	 */
	void ExitBossPhaseCutscene();

	/**
	 * 世界（プレイヤー・ボス・カメラ・ステージのUpdate、エフェクト、サウンド）の更新を止め、描画だけを続ける「一時停止」を切り替える。
	 * チュートリアルでUIAnimation中に「このUIはこういうものです」という説明演出を差し込みたい場合など、
	 * 今の画面をそのまま静止させておきたい場面で使う想定。
	 * FreezePlayerAndBoss()と違い、入力状態のリセットやエフェクト/サウンドの停止(破棄)は行わない——
	 * あくまで「今の見た目のまま時間だけ止める」ための汎用トグル。
	 */
	void SetWorldFrozen(bool freeze);
	/** 現在SetWorldFrozen(true)の状態か */
	inline bool IsWorldFrozen() const { return isWorldFrozen_; }

private:

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
