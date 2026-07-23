/**
 * BattleManager.cpp
 * インゲーム管理
 */

#include "stdafx.h"
#include "BattleManager.h"
#include "MissionManager.h"
#include "src/Effect/EffectManager.h"
#include "src/Sound/SoundManager.h"

#include "Src/Actor/Player.h"
#include "Src/Actor/PlayerController.h"
#include "src/Actor/BossSpawner.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/ActorStatus.h"
#include "src/Actor/AttackObjectManager.h"
#include "src/Stage/StageManager.h"

#include "src/Camera/CameraManager.h"
#include "src/Camera/CameraConfig.h"
#include "src/Camera/CameraSteering.h"

#include "src/collision/GhostBodyManager.h"
#include "src/collision/CollisionHitManager.h"

#include "src/Stage/GrassBendManager.h"
#include "src/UI/Menu.h"
#include "src/UI/BossPhaseCutSceneMenu.h"
#include "src/UI/GameClearMenu.h"
#include "src/UI/GameOverMenu.h"
#include "src/UI/GameStartMenu.h"
#include "src/UI/MissionMenu.h"
#include "src/UI/TimerMenu.h"
#include "../../k2Engine/graphics/DitherCBData.h"


namespace
{
	/* 優先度 */
	constexpr uint8_t PRIORITY_PLAYER            = 0;
	constexpr uint8_t PRIORITY_PLAYER_CONTROLLER = 10;
	constexpr uint8_t PRIORITY_STAGE             = 0;
	constexpr uint8_t PRIORITY_SKYCUBE           = 0;

	/* ディザリング */
	constexpr float DITHERING_ENABLE_TRUE_VALUE  = 1.0f;
	constexpr float DITHERING_ENABLE_FALSE_VALUE = 0.0f;
}

BattleManager* BattleManager::myInstance_ = nullptr;


BattleManager::BattleManager()
{
	// バトル共通パラメータ（カメラ設定・タイマー・カットシーン等の時間や座標）をJSONから取得
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	// プレイヤー
	{
		// プレイヤー本体とその操作コントローラーを生成する
		player_           = NewGO<Player>(PRIORITY_PLAYER, "player");
		playerController_ = NewGO<PlayerController>(PRIORITY_PLAYER_CONTROLLER, "playerController");
		if (playerController_)
		{
			// コントローラーの操作対象をプレイヤーに設定し、ボス登場演出中は入力を受け付けないようにする
			playerController_->SetTarget(player_);
			playerController_->Deactivate();
		}
		if (player_)
		{
			// ステートマシンの入力状態と移動速度を初期化しておく
			auto* stateMachine = player_->GetStateMachine();
			stateMachine->ResetInput();
			player_->SetMoveVelocity(Vector3::Zero);
		}
	}

	// ボス
	{
#if defined(_DEBUG)
		// デバッグ実行時、ステージ選択を経由せず直接起動した場合のフォールバック設定
		if (CharacterDataBase::Get().GetStageType() == BossType::enNone) {
			CharacterDataBase::Get().SetStageType(BossType::enGorilla);
		}
		if (CharacterDataBase::Get().GetGameModeType() == GameModeType::enNone) {
			CharacterDataBase::Get().SetGameModeType(GameModeType::enNormal);
		}
#endif

		// ボス生成・管理クラスを作り、攻撃対象をプレイヤーに設定してボスを生成する
		boss_ = new BossSpawner();
		boss_->SetAttackTarger(player_);
		boss_->SpawnBoss();

		// ボス登場演出が終わるまではAI・移動を止めておく
		boss_->SetControlEnabled(false);
	}

	// キャラクター用設定
	{
		// プレイヤーの装備スキル種別を設定する（特殊能力は現状未装備のまま）
		CharacterDataBase::Get().SetPlayerNormalAttack(NormalAttackType::enBite);
		CharacterDataBase::Get().SetPlayerUtility(UtilityType::enAvoid);

		// 設定した種別に応じたスキル実体をプレイヤーに生成させる
		player_->CreateSkill(
			CharacterDataBase::Get().GetPlayerParam().nAttack,
			CharacterDataBase::Get().GetPlayerParam().ability,
			CharacterDataBase::Get().GetPlayerParam().utility
		);

		// 攻撃オブジェクト（投げた岩・設置した地雷など）を管理するシングルトンを生成する
		AttackObjectManager::CreateInstance();
	}

	// ステージ
	// 静的オブジェクト・コリジョン・草などステージ全体を管理するオブジェクトを生成する
	stage_  = NewGO<StageManagerObject>(PRIORITY_STAGE, "stage");

	// スカイキューブ
	{
		// 空を覆う天球を生成し、種類とスケールを設定する
		skyCube_ = NewGO<SkyCube>(PRIORITY_SKYCUBE, "skyCube");
		skyCube_->SetType(enSkyCubeType_Day);
		skyCube_->SetScale(param->skyCubeScale);
	}

	// カメラ
	{
		// カメラの操作入力（距離・回転速度など）を管理するCameraSteeringを生成する
		cameraSteering_ = std::make_unique<CameraSteering>();

		// カメラ操作設定を組み立てる（距離はCameraConfigに保存済みのユーザー設定値を使う）
		CameraSteering::Config initConfig;
		initConfig.distance       = CameraConfig::Get().GetDistance();
		initConfig.height         = param->cameraParam.height;
		initConfig.rotationSpeedX = param->cameraParam.rotSpeed;
		initConfig.rotationSpeedY = param->cameraParam.rotSpeed;
		initConfig.lookAtOffsetY  = param->cameraParam.lookAtOffsetY;
		// カメラの初期姿勢（視野角・クリップ距離）を組み立てる
		CameraData initData;
		initData.fov      = Math::DegToRad(param->cameraParam.fovy);
		initData.nearClip = param->cameraParam.nearClip;
		initData.farClip  = param->cameraParam.farClip;
		cameraSteering_->SetConfig(initConfig);
		cameraSteering_->SetTargetCharacter(player_);

		// 通常プレイ用カメラ(GameCamera)を生成して初期姿勢を設定する
		auto gameCamera = std::make_shared<GameCamera>();
		gameCamera->SetState(initData);
		gameCameraController_ = gameCamera;

		// CameraManagerに保存された設定値を反映する
		CameraOption initOption;
		initOption.sensitivity = CameraManager::Get().GetSensitivityValue();
		initOption.distance = CameraManager::Get().GetDistance();
		initOption.invert = CameraManager::Get().GetInvert();
		initOption.fovDeg = param->cameraParam.fovy;
		SetCameraOption(initOption);
	}

	// 演出
	{
		// カットシーン用タイマーを生成し、対戦するボスの種類に応じた登場演出をセットアップする
		cutSceneScheduler_ = std::make_unique<TaskSchedulerSystem>();
		BossType stageType = CharacterDataBase::Get().GetStageType();

		if (stageType == BossType::enGorilla) {
			SetupEntryGolliraCutScene();
		}
		if (stageType == BossType::enTurtle) {
			SetupEntryTurtleCutScene();
		}
	}

	// その他
	{
		// 制限時間タイマーを初期化する
		gameTimer_.Init();
		gameTimer_.SetLimitTime(param->gameTimeParam.limitTime);
		gameTimer_.SetWarningTime(param->gameTimeParam.warningTime);

		// ミッション表示用UIを初期化する
		uiManager_.InitMission();

		// ミッション管理シングルトンを生成し、対戦するボスの種類に応じたミッション一式を組み立てる
		MissionManager::CreateInstance();
		MissionManager::Get().InitByBossType(CharacterDataBase::Get().GetStageType());
		// 草曲げ管理シングルトンを生成する
		GrassBendManager::Initialize();

		// ダメージ通知のコールバックを登録する
		// （CollisionHitManagerが被ダメージを検出するたびにこのラムダが呼ばれ、通知キューに積む）
		CollisionHitManager::Get().onDamageNotify = [this](int damage, DamageNotifyType type, bool isCritical)
			{
				PushDamageNotify(damage, type, isCritical);
			};
	}
}


BattleManager::~BattleManager()
{
	// UIManager のデストラクタが Layout を全て解放する

	// このクラスが生成したゲームオブジェクト（プレイヤー・コントローラー・ステージ・スカイキューブ）を破棄する
	DeleteGO(player_);
	DeleteGO(playerController_);
	DeleteGO(stage_);
	DeleteGO(skyCube_);

	// ボス生成・管理クラスを破棄する（内部でボス本体のDeleteGOも行われる）
	if (boss_) {
		delete boss_;
		boss_ = nullptr;
	}

	// このクラスが生成したシングルトンを破棄する
	MissionManager::Get().DestroyInstance();
	GrassBendManager::Finalize();
}


void BattleManager::Update()
{
#ifdef K2_DEBUG
	UpdateDebugGroupInput();
#endif

	switch (gameState_)
	{
		case GameState::Entry:
		{
			// ボス登場カットシーンが再生中ならここで止まる（trueが返る間はEntryのまま）
			if (UpdateEntryBoss()) break;

			// 登場カットシーン終了：そのUIを解放し、続けてゲームスタート演出をセットアップする
			uiManager_.ReleaseCutSceneLayout();
			SetupStartCutScene();
			gameState_ = GameState::GameStart;
			// break を書かずそのままGameStartのcaseへフォールスルーし、同フレームでスタート演出の完了判定も行う
		}
		case GameState::GameStart:
		{
			// スタート演出が完了しているか確認する
			auto* menu = static_cast<GameStartMenu*>(uiManager_.GetCutSceneMenu());
			if (menu && menu->IsCompletedStartCutSceen()) {
				// 演出用UIを解放し、インゲームのタイマーUIをセットアップする
				uiManager_.ReleaseCutSceneLayout();
				uiManager_.InitTimer();

				// プレイヤー操作とボスAIを有効化し、実際のプレイを開始する
				playerController_->Activate();
				boss_->SetControlEnabled(true);
				boss_->SetUpdate(true);
				gameState_ = GameState::Playing;
			}
			break;
		}
		case GameState::Playing:
		{
			// BossEmotionPhaseManager の遅延初期化
			// コンストラクタ時点では BossCharacter::Start() が未完了のため、
			// Playing に入った最初のフレームで Init() する
			if (!bossEmotionPhaseManagerInitialized_)
			{
				auto* boss = boss_->GetBoss();
				auto* bossStatus = boss ? boss->GetStatus()->As<BossStatus>() : nullptr;
				if (bossStatus)
				{
					bossEmotionPhaseManager_.Init(boss, &bossStatus->GetEmotionSystem());
					emotionEffectObserver_.Init(boss, &bossStatus->GetEmotionSystem());
					bossEmotionPhaseManagerInitialized_ = true;
				}
			}
			bossEmotionPhaseManager_.Update(); // ダメージ通知管理
			emotionEffectObserver_.Update(); // 感情システム : 条件により、ボスにバフデバフがかかる
			CheckBossPhaseCutsceneTrigger(); // ボスHP50%/25%でズームアウト演出を開始する
			cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime()); // ボスHP閾値カットシーンのタイマーを進める
			UpdateBossPhaseCutsceneCamera(g_gameTime->GetFrameDeltaTime()); // ボスHP閾値カットシーンのカメライージングを進める
			UpdateBossPhaseCutsceneTilt(g_gameTime->GetFrameDeltaTime()); // HP25%演出：ボスの前後傾き・左右揺れイージングを進める

			gameTimer_.Update();
			boss_->Update();
			GrassBendManager::Get().Update(g_gameTime->GetFrameDeltaTime());

			// ミッションの状態をUIに通知（MissionManager::Update()の前に読む）
			if (auto* missionMenu = uiManager_.GetMissionMenu())
			{
				const int missionCount = MissionManager::Get().GetMissionCount();
				for (int idx = 0; idx < missionCount; idx++)
				{
					auto* mission = MissionManager::Get().GetMissionByIndex(idx); // missionの数を取得
					if (!mission) continue; // missionがないなら処理を終了する

					// 特定のミッションはクリアしているか
					if (mission->IsCleared())
					{
						// クリア済み：アニメーション未再生ならクリア演出を1回だけ再生
						if (!mission->IsClearAnimationPlayed())
						{
							missionMenu->TriggerClear(mission->GetUISlot(), mission->GetCurrentCount());
							mission->SetClearAnimationPlayed();
						}
					}
					// クリアしていないもので、カウントアップさせるタイプのミッションの進捗が進行しているなら
					else if (mission->GetUpdateType() == MissionUpdateType::enCount
						     && mission->IsCountUpdatedThisFrame())
					{
						// カウント増加：このフレームだけtrueなのでUpdate()前に読む
						missionMenu->TriggerMission(mission->GetUISlot(), mission->GetCurrentCount());
					}
				}
			}

			MissionManager::Get().Update();

			// ディザリング設定
			g_ditherCBData.isEnable       = DITHERING_ENABLE_TRUE_VALUE;							// ディザを有効に
			g_ditherCBData.cameraWorldPos = CameraManager::Get().GetCurrentCameraData().position;	// カメラの現在の座標を取得
			g_ditherCBData.playerWorldPos = player_->GetTransformPosition();						// プレイヤーの座標を取得

			// カメラ更新
			auto gameCamera = gameCameraController_->As<GameCamera>();				// 型変換をしてカメラのインターフェースを取得
			auto cameraData = gameCamera->GetCameraData();							// カメラデータを取得
			cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());	// 現在のデータと経過時間をもとに更新
			gameCamera->SetState(cameraData);										// データを設定

			// スカイキューブをプレイヤーに追従
			skyCube_->SetPosition(player_->GetTransformPosition());

			// タイマー UI に値を渡す
			if (auto* timerMenu = uiManager_.GetTimerMenu()) {
				timerMenu->SetRate(gameTimer_.GetRate());
				timerMenu->SetMinutes(gameTimer_.GetRemainMinutes());
				timerMenu->SetSeconds(gameTimer_.GetRemainSeconds());
				timerMenu->SetRequestFlashing(gameTimer_.IsWarningFrame());
			}

			// ボス死亡 → クリア
			if (auto* boss = FindGO<BossCharacter>("boss")) {
				if (boss->GetStatus() && boss->GetStatus()->IsDead()) {
					MissionManager::Get().NotifyBossDefeated();	// ボス撃破のミッション

					SetupClearCutScene();						// クリアカットシーン
					if (player_) {
						auto* stateMachine = player_->GetStateMachine();
						stateMachine->ResetInput();					// ステートマシーンに設定してある入力関係の値をすべてリセット
						player_->SetMoveVelocity(Vector3::Zero);	// プレイヤーの元の移動量を0に
						playerController_->Deactivate();			// プレイヤーが操作できないようにする

					}
					gameState_ = GameState::ResultClear;		// クリアリザルトに移行
				}
			}

			// プレイヤー死亡 / 時間切れ → ゲームオーバー
			if (player_->GetStatus()->IsDead() || gameTimer_.IsTimeUp()) {
				SetupOverCutScene();				// ゲームオーバーカットシーン
				if (player_) {
					auto* stateMachine = player_->GetStateMachine();
					stateMachine->ResetInput();					// ステートマシーンに設定してある入力関係の値をすべてリセット
					player_->SetMoveVelocity(Vector3::Zero);	// プレイヤーの元の移動量を0に
					playerController_->Deactivate();			// プレイヤーが操作できないようにする(回転も止める)
				}
				gameState_ = GameState::ResultOver;	// ゲームオーバーリザルトに移行
			}
			break;
		}
		case GameState::ResultClear:
		{
			if (!UpdateResultClear()) gameState_ = GameState::Shutdown;
			break;
		}
		case GameState::ResultOver:
		{
			if (!UpdateResultOver()) gameState_ = GameState::Shutdown;
			break;
		}
		case GameState::Shutdown:
		{
			// リザルト演出が終わりインゲーム全体を終了する状態：BGM停止・タイマーリセット・ディザ無効化のみ行う
			// （実際のシーン遷移はInGameScene側がIsFinishedGame()を見て行う）
			SoundManager::Get().StopBGM();
			gameTimer_.Reset();
			g_ditherCBData.isEnable = DITHERING_ENABLE_FALSE_VALUE;
			break;
		}
	}

	// ゲーム状態に関わらず毎フレーム更新するもの
	uiManager_.Update();                        // UI全体（メニュー・アニメーション）の更新
	AttackObjectManager::Get().Update();         // 生成済みの攻撃オブジェクト（岩・地雷など）の更新
	GhostBodyManager::Get().Update();            // 当たり判定用ゴーストボディの広域判定更新
	CollisionHitManager::Get().Update();         // Enter/Stay/Exit状態遷移とダメージ処理の更新
	KeyConfig::Get().Update();                   // 長押し判定などキー入力状態の更新
}


void BattleManager::Render(RenderContext& rc)
{
	// UIと攻撃オブジェクトを描画する（本体キャラクター等はGameObjectManager経由で個別に描画される）
	uiManager_.Render(rc);
	AttackObjectManager::Get().Render(rc);
}


void BattleManager::SetGroupMask(const uint32_t updateMask, const uint32_t drawMask)
{
	// 更新・描画のグループマスクを保持し、各オブジェクトへ即座に反映する
	updateMask_ = updateMask;
	drawMask_   = drawMask;
	ApplyGroupMasks();
}


void BattleManager::ApplyGroupMasks()
{
	// updateMask_ の各ビットが「そのグループを更新するか」を表している。
	// (updateMask_ & UpdateGroup::XXX) で対象グループのビットだけを取り出し、
	// 0 以外なら true → SetUpdate(true) でそのオブジェクトの Update を有効にする。
	// 0 なら false → SetUpdate(false) で止める。
	//
	// 例: updateMask_ = 0000 0101 のとき
	//   & Player(0000 0001) → 0000 0001 (≠0) → Player は動く
	//   & Boss  (0000 0010) → 0000 0000 (= 0) → Boss  は止まる
	//   & UI    (0000 0100) → 0000 0100 (≠0) → UI    は動く

	// --- Update 制御 ---
	if (player_)
		player_->SetUpdate((updateMask_ & UpdateGroup::Player) != 0);

	if (boss_)
		boss_->SetUpdate((updateMask_ & UpdateGroup::Boss) != 0);

	if (cameraSteering_)
		cameraSteering_->SetUpdate((updateMask_ & UpdateGroup::Camera) != 0);

	if (stage_)
		stage_->SetUpdate((updateMask_ & UpdateGroup::Stage) != 0);

	// UI だけは Update と Draw を別々のマスクで制御できるため、2行ある。
	// updateMask_ でスクリーンに動くかどうか、drawMask_ で描画するかどうかを独立して管理する。
	// --- UI の Update / Draw 制御 ---
	uiManager_.SetUpdateMask((updateMask_ & UpdateGroup::UI) ? UIManager::All : UIManager::None);
	uiManager_.SetDrawMask  ((drawMask_   & UpdateGroup::UI) ? UIManager::All : UIManager::None);
}


bool BattleManager::UpdateEntryBoss()
{
	// ボス登場カットシーンのタイマー（カメラ切り替え・名前表示など）を1フレーム分進める
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	// Aボタンでカットシーンをスキップ
	if (g_pad[0]->IsTrigger(enButtonA)) {
		isPlayingEntryBoss_ = false;
		cutSceneScheduler_->Reset(); // スキップ時、まだ発火していない演出タイマー（2/3カット目・終了カット）を破棄する
		// 演出用カメラを解除し、通常プレイ用カメラへ即座に切り替える
		CameraManager::Get().Unregister(GameCamera::ID());
		CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
		CameraManager::Get().SwitchCamera(gameCameraController_);
	}

	// falseになった時点でGameState::GameStartへ遷移する（呼び出し元のUpdate()参照）
	return isPlayingEntryBoss_;
}


bool BattleManager::UpdateResultClear()
{
	// クリアリザルト演出のタイマーを1フレーム分進める
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	// クリア演出UIが最後まで再生し終わったら、各種設定を初期値に戻してリザルトを終了する
	auto* menu = dynamic_cast<GameClearMenu*>(uiManager_.GetCutSceneMenu());
	if (menu && menu->IsEnd()) {
		// 初期値に戻る
		CameraManager::Get().ResetToDefault();
		SoundManager::Get().ResetToDefault();
		KeyConfig::Get().ResetToDefault();

		isPlayingResult_ = false;
	}

	// falseになった時点でGameState::Shutdownへ遷移する（呼び出し元のUpdate()参照）
	return isPlayingResult_;
}


bool BattleManager::UpdateResultOver()
{
	// ゲームオーバーリザルト演出のタイマーを1フレーム分進める
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	// ゲームオーバー演出UIが最後まで再生し終わったら、各種設定を初期値に戻してリザルトを終了する
	auto* menu = dynamic_cast<GameOverMenu*>(uiManager_.GetCutSceneMenu());
	if (menu && menu->IsEnd()) {
		// 初期値に戻る
		CameraManager::Get().ResetToDefault();
		SoundManager::Get().ResetToDefault();
		KeyConfig::Get().ResetToDefault();

		isPlayingResult_ = false;
	}

	// falseになった時点でGameState::Shutdownへ遷移する（呼び出し元のUpdate()参照）
	return isPlayingResult_;
}


void BattleManager::SetupEntryGolliraCutScene()
{
	// カットシーン用の共通パラメータ（カメラ設定・各カットの時間/位置）を取得
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	// 登場演出再生中フラグを立てる（UpdateEntryBoss()がfalseになるまでこの演出を続ける）
	isPlayingEntryBoss_ = true;

	// カットシーン UI を生成
	uiManager_.SetupCutScene<MenuBase>("Assets/ui/layout/BossEntryCutScene.json");

	// 演出カメラを生成し、通常プレイ用カメラから切り替える
	CameraData initData;
	initData.fov     = Math::DegToRad(param->cameraParam.fovy);
	initData.farClip = param->cameraParam.farClip;
	auto gameCamera  = std::make_shared<GameCamera>();
	gameCamera->SetState(initData);
	bossEntryCameraController_ = gameCamera;

	CameraManager::Get().Register(GameCamera::ID(), bossEntryCameraController_);
	CameraManager::Get().SwitchCamera(bossEntryCameraController_);

	// カットシーンスケジューラー: 各カットの切り替わりタイミングでカメラ位置とUI表示を変える
	// 1カット目: カメラを最初の位置へ、ゴリラ本体と名前(A)を表示する
	cutSceneScheduler_->AddTimer(param->cutSceneParam.firstCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.firstCutCameraPos;
			cameraData.target   = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("gollira"))->isDraw        = true;
			menu->GetUI<UIIcon>(Hash32("gollira_nameA"))->isDraw  = true;
		});

	// 2カット目: カメラを2番目の位置へ、名前(C)を追加表示する
	cutSceneScheduler_->AddTimer(param->cutSceneParam.secondCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.secondCutCameraPos;
			cameraData.target   = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("gollira_nameC"))->isDraw = true;
		});

	// 3カット目: カメラを3番目の位置へ、名前(B)を追加表示する
	cutSceneScheduler_->AddTimer(param->cutSceneParam.thirdCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.thirdCutCameraPos;
			cameraData.target   = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("gollira_nameB"))->isDraw = true;
		});

	// 終了カット: 演出を終了し、カメラを通常プレイ用に戻す。カットシーンUIを非表示にする
	cutSceneScheduler_->AddTimer(param->cutSceneParam.endCutTime, [this]()
		{
			isPlayingEntryBoss_ = false;
			CameraManager::Get().Unregister(GameCamera::ID());
			CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
			CameraManager::Get().SwitchCamera(gameCameraController_);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("gollira_nameB"))->isDraw = false;
			menu->GetUI<UIIcon>(Hash32("gollira"))->isDraw       = false;
		});
}

void BattleManager::SetupEntryTurtleCutScene()
{
	// カットシーン用の共通パラメータ（カメラ設定・各カットの時間/位置）を取得
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	// 登場演出再生中フラグを立てる（UpdateEntryBoss()がfalseになるまでこの演出を続ける）
	isPlayingEntryBoss_ = true;

	// カットシーン UI を生成
	uiManager_.SetupCutScene<MenuBase>("Assets/ui/layout/BossEntryCutScene.json");

	// 演出カメラを生成し、通常プレイ用カメラから切り替える
	CameraData initData;
	initData.fov = Math::DegToRad(param->cameraParam.fovy);
	initData.farClip = param->cameraParam.farClip;
	auto gameCamera = std::make_shared<GameCamera>();
	gameCamera->SetState(initData);
	bossEntryCameraController_ = gameCamera;

	CameraManager::Get().Register(GameCamera::ID(), bossEntryCameraController_);
	CameraManager::Get().SwitchCamera(bossEntryCameraController_);

	// カットシーンスケジューラー: 各カットの切り替わりタイミングでカメラ位置とUI表示を変える
	// 1カット目: カメラを最初の位置へ、カメ本体と名前(A)を表示する
	cutSceneScheduler_->AddTimer(param->cutSceneParam.firstCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.firstCutCameraPos;
			cameraData.target = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("turtle"))->isDraw = true;
			menu->GetUI<UIIcon>(Hash32("turtle_nameA"))->isDraw = true;
		});

	// 2カット目: カメラを2番目の位置へ、名前(C)を追加表示する
	cutSceneScheduler_->AddTimer(param->cutSceneParam.secondCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.secondCutCameraPos;
			cameraData.target = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("turtle_nameC"))->isDraw = true;
		});

	// 3カット目: カメラを3番目の位置へ、名前(B)を追加表示する
	cutSceneScheduler_->AddTimer(param->cutSceneParam.thirdCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.thirdCutCameraPos;
			cameraData.target = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("turtle_nameB"))->isDraw = true;
		});

	// 終了カット: 演出を終了し、カメラを通常プレイ用に戻す。カットシーンUIを非表示にする
	cutSceneScheduler_->AddTimer(param->cutSceneParam.endCutTime, [this]()
		{
			isPlayingEntryBoss_ = false;
			CameraManager::Get().Unregister(GameCamera::ID());
			CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
			CameraManager::Get().SwitchCamera(gameCameraController_);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("turtle_nameB"))->isDraw = false;
			menu->GetUI<UIIcon>(Hash32("turtle"))->isDraw = false;
		});
}


void BattleManager::SetupStartCutScene()
{
	// ゲームスタート演出用UIをセットアップする（演出終了はGameStartMenu::IsCompletedStartCutSceen()側で判定）
	uiManager_.SetupCutScene<GameStartMenu>("Assets/ui/layout/GameStartMenu.json");
}


void BattleManager::SetupClearCutScene()
{
	isPlayingResult_ = true; // リザルトを再生したフラグをtrueに
	cutSceneScheduler_->Reset(); // カットシーンのタスクスケジューラーをリセット

	SoundManager::Get().PlayBGM(enSoundKind_GameClear); // ゲームクリア時のBGMを再生

	// スタックしてあるミッションのuiAnimationを全て破棄
	if (auto* missionMenu = uiManager_.GetMissionMenu())
	{
		missionMenu->ClearNotificationQueue();
	}

	// ゲームクリア時のuiAnimationをセットアップ
	uiManager_.SetupCutScene<GameClearMenu>("Assets/ui/layout/GameClearMenu.json");

	// リザルト中はタイマーを非表示
	if (auto* timerMenu = uiManager_.GetTimerMenu())
	{
		timerMenu->GetCanvas()->isDraw = false;
	}
}


void BattleManager::SetupOverCutScene()
{
	isPlayingResult_ = true; // リザルトを再生したフラグをtrueに
	cutSceneScheduler_->Reset(); // カットシーンのタスクスケジューラーをリセット

	// ゲームオーバーのBGMを再生
	SoundManager::Get().PlayBGM(enSoundKind_GameOver);
	SoundManager::Get().SetBGMVolumeScale(1.5f); // ボリューム調整

	// スタックしてあるミッションのuiAnimationを全て破棄
	if (auto* missionMenu = uiManager_.GetMissionMenu())
	{
		missionMenu->ClearNotificationQueue();
	}

	// ゲームオーバー時のuiAnimationをセットアップ
	uiManager_.SetupCutScene<GameOverMenu>("Assets/ui/layout/GameOverMenu.json");
}


void BattleManager::CheckBossPhaseCutsceneTrigger()
{
	auto* boss = boss_->GetBoss();
	auto* bossStatus = boss ? boss->GetStatus()->As<BossStatus>() : nullptr;
	if (!bossStatus) { return; }

	// ボスの種類ごとの閾値をマスターデータから取得（見つからない場合は構造体のデフォルト値を使う）
	MasterBossPhaseCutSceneParameter defaultCutSceneParam;
	const BossType stageType = CharacterDataBase::Get().GetStageType();
	const auto* cutSceneParam = ParameterManager::Get().GetBossPhaseCutSceneParam(stageType == BossType::enGorilla ? "Gorilla" : "Turtle");
	if (!cutSceneParam) { cutSceneParam = &defaultCutSceneParam; }

	// HPの割合を計算
	const float hpRatio = static_cast<float>(bossStatus->GetHP()) / static_cast<float>(bossStatus->GetMaxHP());

	// HPが特定の割合を下回ったとき、怒り状態になる。(一度だけ)
	if (!bossPhase50CutsceneTriggered_ && hpRatio <= cutSceneParam->angryHpRatio) {
		bossPhase50CutsceneTriggered_ = true;
		EnterBossPhaseCutscene(false);
	}

	// HPが特定の割合を下回ったとき、疲れた状態になる。(一度だけ)
	if (!bossPhase25CutsceneTriggered_ && hpRatio <= cutSceneParam->tiredHpRatio) {
		bossPhase25CutsceneTriggered_ = true;
		EnterBossPhaseCutscene(true);
	}
}


void BattleManager::EnterBossPhaseCutscene(const bool isPhase25)
{
	// プレイヤー・ボスを操作不能にし、再生中のエフェクト/サウンドを止める（他のカットシーンとも共通の汎用処理）
	FreezePlayerAndBoss();

	// プレイヤーが操作可能に戻るまでtrue。InGameSceneはこれを見てインゲームHUD（HPバー・攻撃方法など）の描画/更新を止める
	isBossPhaseCutsceneActive_ = true;

	// カットシーン中はプレイヤーモデルの描画も止める（カメラがボス正面に寄るため、画面内にプレイヤーが映り込まないように）
	if (player_) { player_->SetDraw(false); }

	// 怒りマークなど、このカットシーン専用のUI（初期状態は全部isDraw:false）をセットアップする
	// アイコンのオフセット適用・UIAnimationのアタッチは BossPhaseCutSceneMenu::InitializeLogic() 側で行う
	uiManager_.SetupCutScene<BossPhaseCutSceneMenu>("Assets/ui/layout/BossPhaseCutSceneUI.json");

	// 演出用タイマーをリセットしてから使う
	// （BossPhaseCutSceneMenu側は SetupCutScene() のたびに新規生成されるため、専用スケジューラーのリセットは不要）
	cutSceneScheduler_->Reset();

	auto* boss = boss_->GetBoss();
	if (!boss) { return; }

	// カメラ位置・演出内容はボスの種類ごとに異なるため、パラメータ取得の時点からボス別の関数に分ける
	const BossType stageType = CharacterDataBase::Get().GetStageType();
	if (stageType == BossType::enGorilla) {
		SetupGorillaPhaseCutsceneCamera(boss, isPhase25);
	}
	else if (stageType == BossType::enTurtle) {
		SetupTurtlePhaseCutsceneCamera(boss, isPhase25);
	}
}


void BattleManager::SetupGorillaPhaseCutsceneCamera(BossCharacter* boss, const bool isPhase25)
{
	// ゴリラ用のカメラ・演出パラメータをマスターデータから取得（見つからない場合は構造体のデフォルト値を使う）
	MasterBossPhaseCutSceneParameter defaultCutSceneParam;
	const auto* cutSceneParam = ParameterManager::Get().GetBossPhaseCutSceneParam("Gorilla");
	if (!cutSceneParam) { cutSceneParam = &defaultCutSceneParam; }

	// ボスの座標を取得
	const Vector3 bossPos = boss->GetTransformPosition();

	// ボスが現在向いている方向（プレイヤーへの方向ではなく、ボス自身の回転から求める。
	// 攻撃行動終了後などはプレイヤーの方を向いているとは限らないため）
	// BossState.cpp の SetRotationY(atan2(diff.x, diff.z)) と同じ規約で、ローカル正面はワールド+Z
	Vector3 bossForwardDir(0.0f, 0.0f, 1.0f);
	boss->GetTransformRotation().Apply(bossForwardDir);
	bossForwardDir.y = 0.0f; // 水平方向のみで向きを計算する
	if (bossForwardDir.LengthSq() > 0.001f) { bossForwardDir.Normalize(); }
	else { bossForwardDir = Vector3(0.0f, 0.0f, 1.0f); }

	// 現在のゲームカメラのfov/クリップ距離を引き継ぎ、位置と注視点だけ差し替える
	CameraData cutsceneCameraData = gameCameraController_->As<GameCamera>()->GetCameraData();
	// カメラ位置 = ボス位置 から「ボスの正面方向」に cameraDistance 分離し、cameraHeight 分だけ上げた場所
	cutsceneCameraData.position = bossPos + bossForwardDir * cutSceneParam->cameraDistance + Vector3(0.0f, cutSceneParam->cameraHeight, 0.0f);
	// 注視点 = ボスの位置（同じ高さオフセットを加えてボスの顔付近を見るようにする）
	cutsceneCameraData.target   = bossPos + Vector3(0.0f, cutSceneParam->cameraHeight, 0.0f);

	StartBossPhaseCutsceneCamera(boss, isPhase25, cutsceneCameraData, *cutSceneParam);
}


void BattleManager::SetupTurtlePhaseCutsceneCamera(BossCharacter* boss, const bool isPhase25)
{
	// カメ用のカメラ・演出パラメータをマスターデータから取得（見つからない場合は構造体のデフォルト値を使う）
	MasterBossPhaseCutSceneParameter defaultCutSceneParam;
	const auto* cutSceneParam = ParameterManager::Get().GetBossPhaseCutSceneParam("Turtle");
	if (!cutSceneParam) { cutSceneParam = &defaultCutSceneParam; }

	const Vector3 bossPos = boss->GetTransformPosition();

	// ボスが現在向いている方向（プレイヤーへの方向ではなく、ボス自身の回転から求める。
	// 攻撃行動終了後などはプレイヤーの方を向いているとは限らないため）
	// BossState.cpp の SetRotationY(atan2(diff.x, diff.z)) と同じ規約で、ローカル正面はワールド+Z
	Vector3 bossForwardDir(0.0f, 0.0f, 1.0f);
	boss->GetTransformRotation().Apply(bossForwardDir);
	bossForwardDir.y = 0.0f; // 水平方向のみで向きを計算する（上下の傾きは無視）
	if (bossForwardDir.LengthSq() > 0.001f) { bossForwardDir.Normalize(); }
	else { bossForwardDir = Vector3(0.0f, 0.0f, 1.0f); }

	// 現在のゲームカメラのfov/クリップ距離を引き継ぎ、位置と注視点だけ差し替える
	CameraData cutsceneCameraData = gameCameraController_->As<GameCamera>()->GetCameraData();
	// カメラ位置 = ボス位置 から「ボスの正面方向」に cameraDistance 分離し、cameraHeight 分だけ上げた場所
	cutsceneCameraData.position = bossPos + bossForwardDir * cutSceneParam->cameraDistance + Vector3(0.0f, cutSceneParam->cameraHeight, 0.0f);
	// 注視点 = ボスの位置（同じ高さオフセットを加えてボスの顔付近を見るようにする）
	cutsceneCameraData.target   = bossPos + Vector3(0.0f, cutSceneParam->cameraHeight, 0.0f);

	StartBossPhaseCutsceneCamera(boss, isPhase25, cutsceneCameraData, *cutSceneParam);
}


void BattleManager::StartBossPhaseCutsceneCamera(BossCharacter* boss, const bool isPhase25, const CameraData& cutsceneCameraData, const MasterBossPhaseCutSceneParameter& cutSceneParam)
{
	// タイマーのラムダで使う値をローカル変数にコピーしておく（cutSceneParamの参照そのものをキャプチャせずに済むように）
	const float cameraEaseDuration    = cutSceneParam.cameraEaseDuration;
	const float cutsceneDuration      = cutSceneParam.cutsceneDuration;
	const float bossIdleHoldDuration  = cutSceneParam.bossIdleHoldDuration;

	// イージングの開始地点＝現在の通常カメラ（演出終了後に戻ってくる先でもある）
	bossPhaseCutsceneCameraStartData_  = gameCameraController_->As<GameCamera>()->GetCameraData();
	// イージングの到達地点＝呼び出し元(Setup*PhaseCutsceneCamera)で計算したボス正面カメラ
	bossPhaseCutsceneCameraTargetData_ = cutsceneCameraData;

	// カットシーン用カメラは、まず現在の通常カメラと同じ状態で生成する（切り替えた瞬間に映像が飛ばないように）
	auto bossFrontCamera = std::make_shared<GameCamera>();
	bossFrontCamera->SetState(bossPhaseCutsceneCameraStartData_);
	bossEntryCameraController_ = bossFrontCamera;

	// 通常のゲームカメラをカットシーン用カメラに差し替えて表示を切り替える
	CameraManager::Get().Unregister(GameCamera::ID());
	CameraManager::Get().Register(GameCamera::ID(), bossEntryCameraController_);
	CameraManager::Get().SwitchCamera(bossEntryCameraController_);

	// 0→1にイージングさせ、その進捗値でstart/targetのカメラをブレンドする
	// （瞬間移動ではなく、cameraEaseDuration秒かけてボス正面へ寄っていく。EaseOutで初動を速く、後半をゆっくりにする）
	bossPhaseCutsceneCameraBlendCurve_.Initialize(0.0f, 1.0f, cameraEaseDuration, EasingType::EaseOut);
	bossPhaseCutsceneCameraBlendCurve_.Play();

	// HP25%演出: ボスの前後傾き（X軸回転）のイージング秒数・角度（BossPhaseCutSceneParameter.jsonで調整）
	// 0→+pitchForwardDeg(pitchForwardEaseDuration秒) → 静止(pitchHoldDuration秒) → +pitchForwardDeg→0度(pitchBackEaseDuration秒)
	const float pitchForwardDeg         = cutSceneParam.pitchForwardDeg;
	const float pitchForwardEaseDuration = cutSceneParam.pitchForwardEaseDuration;
	const float pitchHoldDuration        = cutSceneParam.pitchHoldDuration;
	const float pitchBackEaseDuration    = cutSceneParam.pitchBackEaseDuration;

	// カメラがボス正面に到着したタイミングで、専用演出を開始する
	// 50%は通常攻撃アニメーションの流用、25%はX軸回転のイージングのみ。見え方次第で専用アニメーションに差し替える
	cutSceneScheduler_->AddTimer(cameraEaseDuration, [this, boss, isPhase25, pitchForwardDeg, pitchForwardEaseDuration, cutSceneParam]()
		{
			if (isPhase25)
			{
				// HP25%用: 現在の向きを基準に、前方向へpitchForwardDeg度イージングさせる（角度・秒数はJSON側で調整）
				if (boss)
				{
					bossPhaseCutscenePitchBaseRotation_ = boss->GetTransformRotation();
					bossPhaseCutscenePitchCurve_.Initialize(0.0f, pitchForwardDeg, pitchForwardEaseDuration, EasingType::EaseInOut);
					bossPhaseCutscenePitchCurve_.Play();
				}
				// 疲れマークの表示開始はUI側(BossPhaseCutSceneMenu::StartTiredMarks)に委譲する（後述のtiredStartTimeタイマーで呼び出す）

				// カメラがボス正面に到着したこのタイミングでデバフ（Debuff3）とエフェクトを発生させる
				// （HP25%を跨いだ瞬間ではなく、カットシーンの演出に同期させる）
				// このカットシーンのエフェクトだけ再生位置を少し下げる（通常のインゲームDebuffエフェクトの位置はEmotionParameter.jsonのまま変えない）
				// サウンドはEmotionEffectObserver::OnEmotionChanged()側で共通再生される（カットシーン以外のバフ/デバフ発生時も鳴る）
				emotionEffectObserver_.SetNextDebuffEffectOffsetY(cutSceneParam.debuffEffectOffsetYOverride);
				bossEmotionPhaseManager_.ForceApplyPhase25Debuff();
			}
			else
			{
				// HP50%用: ジャンプ→着地をbossJumpCount回行う（通常攻撃アニメーションは一旦破棄）
				// 1回目の着地はUpdateBossPhaseCutsceneCamera()側で検知して次のジャンプにつなげ、
				// bossJumpCount回終わったらIdleに戻す
				if (boss) {
					boss->PlayAnimation(BossAnimID::enAnimJump);
					bossPhaseCutsceneJumpsRemaining_ = cutSceneParam.bossJumpCount;
				}

				// カメラがボス正面に到着したこのタイミングでバフ（Buff3）とエフェクトを発生させる
				// （HP50%を跨いだ瞬間ではなく、カットシーンの演出に同期させる）
				// このカットシーンのエフェクトだけ再生位置を少し上げる（通常のインゲームBuffエフェクトの位置はEmotionParameter.jsonのまま変えない）
				// サウンドはEmotionEffectObserver::OnEmotionChanged()側で共通再生される（カットシーン以外のバフ/デバフ発生時も鳴る）
				emotionEffectObserver_.SetNextBuffEffectOffsetY(cutSceneParam.buffEffectOffsetYOverride);
				bossEmotionPhaseManager_.ForceApplyPhase50Buff();

				// 怒りマーク表示・スケールパルスの開始はUI側(BossPhaseCutSceneMenu)の自前タスクスケジューラーに任せる
				if (auto* menu = GetBossPhaseCutSceneMenu()) { menu->StartAngryMarks(cutSceneParam); }
			}
		});

	// HP25%演出: 前傾きで静止していた後、元の角度(0度)へイージングで戻す（試作）
	if (isPhase25)
	{
		cutSceneScheduler_->AddTimer(cameraEaseDuration + pitchForwardEaseDuration + pitchHoldDuration, [this, pitchForwardDeg, pitchBackEaseDuration]()
			{
				bossPhaseCutscenePitchCurve_.Initialize(pitchForwardDeg, 0.0f, pitchBackEaseDuration, EasingType::EaseInOut);
				bossPhaseCutscenePitchCurve_.Play();
			});
	}

	// HP25%演出: 前傾きで静止している間(pitchHoldDuration秒)に、左右へ小刻みに揺らす（試作）
	// 左へA度→右へ2A度→左へ2A度→右へA度、と4等分した秒数ごとにイージングさせると、ちょうどpitchHoldDuration秒で中央(0度)に戻る
	if (isPhase25)
	{
		const float yawWobbleAmplitudeDeg  = cutSceneParam.yawWobbleAmplitudeDeg;
		const float yawSegmentDuration     = pitchHoldDuration / 4.0f;
		const float holdStartTime          = cameraEaseDuration + pitchForwardEaseDuration;

		cutSceneScheduler_->AddTimer(holdStartTime, [this, yawWobbleAmplitudeDeg, yawSegmentDuration]()
			{
				// 中央(0度) → 左A度
				bossPhaseCutsceneYawCurve_.Initialize(0.0f, -yawWobbleAmplitudeDeg, yawSegmentDuration, EasingType::EaseInOut);
				bossPhaseCutsceneYawCurve_.Play();
			});
		cutSceneScheduler_->AddTimer(holdStartTime + yawSegmentDuration, [this, yawWobbleAmplitudeDeg, yawSegmentDuration]()
			{
				// 左A度 → 右2A度分移動（絶対角度としては右A度の位置）
				bossPhaseCutsceneYawCurve_.Initialize(-yawWobbleAmplitudeDeg, yawWobbleAmplitudeDeg, yawSegmentDuration, EasingType::EaseInOut);
				bossPhaseCutsceneYawCurve_.Play();
			});
		cutSceneScheduler_->AddTimer(holdStartTime + yawSegmentDuration * 2.0f, [this, yawWobbleAmplitudeDeg, yawSegmentDuration]()
			{
				// 右A度 → 左2A度分移動（絶対角度としては左A度の位置）
				bossPhaseCutsceneYawCurve_.Initialize(yawWobbleAmplitudeDeg, -yawWobbleAmplitudeDeg, yawSegmentDuration, EasingType::EaseInOut);
				bossPhaseCutsceneYawCurve_.Play();
			});
		cutSceneScheduler_->AddTimer(holdStartTime + yawSegmentDuration * 3.0f, [this, yawWobbleAmplitudeDeg, yawSegmentDuration]()
			{
				// 左A度 → 右A度分移動（中央=0度に戻る）
				bossPhaseCutsceneYawCurve_.Initialize(-yawWobbleAmplitudeDeg, 0.0f, yawSegmentDuration, EasingType::EaseInOut);
				bossPhaseCutsceneYawCurve_.Play();
			});
	}

	// HP25%演出: ボスが左右への揺れ（Y軸回転）を始めてから戻り始める直前までの間(pitchHoldDuration秒)に、
	// 疲れマーク1→2→3の順にtiredStaggerInterval秒間隔で開始する。
	// 各マークは弧の上昇(tiredArcRiseDuration秒)→横方向への移動(UIAnimation.json側のtiredN_arcDriftの秒数)の2段階で移動しつつ、
	// 同時に透明度もフェードアウトさせる（BossPhaseCutSceneParameter.jsonで調整）
	if (isPhase25)
	{
		// ボスが左右への揺れ（Y軸回転）を始めるタイミング（=前傾きイージングが終わったタイミング）を基準にする
		const float tiredStartTime = cameraEaseDuration + pitchForwardEaseDuration;

		// 疲れマーク1→2→3の順次表示・弧移動・フェードアウトはUI側(BossPhaseCutSceneMenu)の自前タスクスケジューラーに委譲する
		cutSceneScheduler_->AddTimer(tiredStartTime, [this, cutSceneParam]()
			{
				if (auto* menu = GetBossPhaseCutSceneMenu()) { menu->StartTiredMarks(cutSceneParam); }
			});
	}

	// cutsceneDuration秒間演出を見せたら、カメラを戻し始めるのに合わせてマークも消す
	cutSceneScheduler_->AddTimer(cameraEaseDuration + cutsceneDuration, [this, cameraEaseDuration, isPhase25, cutSceneParam]()
		{
			// target→startへイージングし直す（＝元の視点にcameraEaseDuration秒かけて戻す。こちらも初動を速く、後半をゆっくりにする）
			bossPhaseCutsceneCameraBlendCurve_.Initialize(1.0f, 0.0f, cameraEaseDuration, EasingType::EaseOut);
			bossPhaseCutsceneCameraBlendCurve_.Play();

			// マークの非表示化・パルス停止・スケールリセットはUI側(BossPhaseCutSceneMenu)に任せる
			if (auto* menu = GetBossPhaseCutSceneMenu()) { menu->EndMarks(isPhase25, cutSceneParam); }
		});

	// カメラが完全に戻った後、通常カメラへ切り替えてプレイヤーの操作・描画を復帰させる
	cutSceneScheduler_->AddTimer(cameraEaseDuration + cutsceneDuration + cameraEaseDuration, [this]()
		{
			// カットシーン用カメラを外し、通常のゲームカメラへ登録し直して切り替える
			CameraManager::Get().Unregister(GameCamera::ID());
			CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
			CameraManager::Get().SwitchCamera(gameCameraController_);

			// プレイヤーの操作・描画を復帰させる
			if (playerController_) { playerController_->Activate(); }
			if (player_) { player_->SetDraw(true); }

			// プレイヤーが動けるようになったのと同時にインゲームUI（タイマー・HPバー・攻撃方法など）の描画/更新を再開する
			uiManager_.SetUpdateMask(UIManager::All);
			uiManager_.SetDrawMask(UIManager::All);
			uiManager_.ReleaseCutSceneLayout(); // 怒りマークなど、このカットシーン専用のUIを解放する
			isBossPhaseCutsceneActive_ = false;

			// プレイヤーが動けるようになったのと同時にゲーム内時間（制限時間タイマー）も再開する
			SetPouse(false);
		});

	// 視点が戻ってからさらにbossIdleHoldDuration秒待ってからボスAIを再開する（戻った瞬間に攻撃が当たって見えるのを防ぐ）
	// ExitBossPhaseCutscene() 側でボスAI再開やフラグの後始末を行う
	cutSceneScheduler_->AddTimer(cameraEaseDuration + cutsceneDuration + cameraEaseDuration + bossIdleHoldDuration, [this]()
		{
			ExitBossPhaseCutscene();
		});
}


void BattleManager::UpdateBossPhaseCutsceneCamera(const float deltaTime)
{
	// HP50%演出：ジャンプの着地（＝アニメーション再生終了）を検知したら、次のジャンプへつなげる。
	// 2回終わったら待機アニメーションに戻す
	if (bossPhaseCutsceneJumpsRemaining_ > 0)
	{
		if (auto* boss = boss_->GetBoss())
		{
			if (!boss->IsPlayingAnimation())
			{
				bossPhaseCutsceneJumpsRemaining_--;
				if (bossPhaseCutsceneJumpsRemaining_ > 0) {
					boss->PlayAnimation(BossAnimID::enAnimJump); // 2回目のジャンプ
				} else {
					boss->PlayAnimation(BossAnimID::enAnimIdle); // 2回終わったので待機に戻す
				}
			}
		}
	}

	// イージング中でなければ何もしない（カットシーンを開始していない/イージングが完了している）
	if (!bossPhaseCutsceneCameraBlendCurve_.IsPlaying()) { return; }

	bossPhaseCutsceneCameraBlendCurve_.Update(deltaTime);

	// 進捗値(0〜1)に応じてstart/targetのカメラデータをブレンドする
	const float t = bossPhaseCutsceneCameraBlendCurve_.GetCurrentValue();
	const CameraData blended = CameraData::Lerp(t, bossPhaseCutsceneCameraStartData_, bossPhaseCutsceneCameraTargetData_);

	if (bossEntryCameraController_)
	{
		bossEntryCameraController_->As<GameCamera>()->SetState(blended);
	}
}


void BattleManager::UpdateBossPhaseCutsceneTilt(const float deltaTime)
{
	const bool isPitchPlaying = bossPhaseCutscenePitchCurve_.IsPlaying();
	const bool isYawPlaying   = bossPhaseCutsceneYawCurve_.IsPlaying();

	// どちらのイージングも進行中でなければ何もしない（HP25%演出を開始していない/両方とも完了している）
	if (!isPitchPlaying && !isYawPlaying) { return; }

	// 進行中の方だけカーブを進め、直近の角度(度)を更新する（停止中の軸は直前の角度を保持し続ける）
	if (isPitchPlaying)
	{
		bossPhaseCutscenePitchCurve_.Update(deltaTime);
		bossPhaseCutscenePitchCurrentDeg_ = bossPhaseCutscenePitchCurve_.GetCurrentValue();
	}
	if (isYawPlaying)
	{
		bossPhaseCutsceneYawCurve_.Update(deltaTime);
		bossPhaseCutsceneYawCurrentDeg_ = bossPhaseCutsceneYawCurve_.GetCurrentValue();
	}

	// 前後(X)・左右(Y)の角度(度)を基準の向きに加えて反映する
	if (auto* boss = boss_->GetBoss())
	{
		boss->SetCutsceneTiltOffset(bossPhaseCutscenePitchBaseRotation_, Math::DegToRad(bossPhaseCutscenePitchCurrentDeg_), Math::DegToRad(bossPhaseCutsceneYawCurrentDeg_));
	}
}


BossPhaseCutSceneMenu* BattleManager::GetBossPhaseCutSceneMenu() const
{
	return dynamic_cast<BossPhaseCutSceneMenu*>(uiManager_.GetCutSceneMenu());
}


void BattleManager::FreezePlayerAndBoss()
{
	// プレイヤーの処理
	{
		// ステートをリセット
		if (player_) {
			auto* stateMachine = player_->GetStateMachine();
			stateMachine->ResetInput();				// ステートマシーンに設定してある入力関係の値をすべてリセット
			player_->SetMoveVelocity(Vector3::Zero);	// プレイヤーの元の移動量を0に
		}
		// 操作不能
		if (playerController_) { playerController_->Deactivate(); }
	}

	// ボスのAIを止める（アニメーション自体の更新は継続するので、こちらから ChangeState/PlayAnimation を呼べる）
	if (boss_) { boss_->SetControlEnabled(false); }

	// 再生中のエフェクト・サウンドをすべて止める
	EffectManager::Get().StopAllEffects();
	SoundManager::Get().StopAllSE();

	// インゲームUI（タイマー・HPバー・攻撃方法など）の更新・描画を止める
	// CutSceneビットだけは残す（このカットシーン専用の怒りマークなどをここで表示するため）
	uiManager_.SetUpdateMask(UIManager::CutScene);
	uiManager_.SetDrawMask(UIManager::CutScene);

	// ゲーム内時間（制限時間タイマー）を止める
	SetPouse(true);
}


void BattleManager::UnfreezePlayerAndBoss()
{
	if (playerController_) { playerController_->Activate(); }

	if (boss_) {
		// ボスのAIを再開する前に、必ずIdle状態から始まるようにする
		if (auto* boss = boss_->GetBoss()) { boss->ChangeState(BossStateID::Idle); }
		boss_->SetControlEnabled(true);
	}

	// インゲームUIの更新・描画を元に戻す
	uiManager_.SetUpdateMask(UIManager::All);
	uiManager_.SetDrawMask(UIManager::All);

	// ゲーム内時間（制限時間タイマー）を再開する
	SetPouse(false);
}


void BattleManager::ExitBossPhaseCutscene()
{
	// プレイヤー操作・ボスAI・UI更新・ゲーム内時間をすべて元に戻す
	UnfreezePlayerAndBoss();
}


#ifdef K2_DEBUG
void BattleManager::UpdateDebugGroupInput()
{
	// F1、F2〜F5 のトリガー検出
	static bool prev[4] = {};

	struct Entry { int vKey; uint32_t group; const char* label; };
	constexpr Entry table[4] = {
		{ VK_F1, UpdateGroup::Player, "Player" },
		{ VK_F5, UpdateGroup::Boss,   "Boss"   },
		{ VK_F3, UpdateGroup::UI,     "UI"     },
		{ VK_F4, UpdateGroup::Stage,  "Stage"  },
	};

	bool changed = false;
	for (int i = 0; i < 4; i++) {
		// GetAsyncKeyState はキーの状態を 16 ビット整数で返す。
		// 最上位ビット（0x8000 = 1000 0000 0000 0000）が 1 なら「今このキーが押されている」。
		// & 0x8000 でそのビットだけ取り出し、0 以外なら押下中と判断する。
		const bool cur = (GetAsyncKeyState(table[i].vKey) & 0x8000) != 0;
		if (cur && !prev[i]) {
			// ^= (XOR) でビットをトグルする。
			// 対象グループのビットが 1 なら 0 に、0 なら 1 に反転する。
			// つまり F キーを押すたびに ON → OFF → ON と切り替わる。
			updateMask_ ^= table[i].group;  // トグル
			changed = true;

			char buf[64];
			// トグル後に & で対象ビットを取り出し、今 ON か OFF かをデバッグ表示する。
			const bool isOn = (updateMask_ & table[i].group) != 0;
			sprintf_s(buf, "[Debug] %s Update: %s\n", table[i].label, isOn ? "ON" : "OFF");
			OutputDebugStringA(buf);
		}
		prev[i] = cur;
	}

	if (changed) ApplyGroupMasks();

	// F6 : エフェクト更新 ON/OFF（Effekseer を一時停止）
	static bool prevF6 = false;
	const bool curF6 = (GetAsyncKeyState(VK_F6) & 0x8000) != 0;
	if (curF6 && !prevF6) {
		const bool isPaused = !EffectManagerObject::IsUpdatePaused();
		EffectManagerObject::SetUpdatePaused(isPaused);
		char buf[64];
		sprintf_s(buf, "[Debug] Effect Update: %s\n", isPaused ? "OFF" : "ON");
		OutputDebugStringA(buf);
	}
	prevF6 = curF6;

	// F7 : サウンド一時停止 ON/OFF（BGM・ループSEをその場で止める）
	static bool prevF7 = false;
	const bool curF7 = (GetAsyncKeyState(VK_F7) & 0x8000) != 0;
	if (curF7 && !prevF7) {
		const bool isPaused = !SoundManagerObject::IsUpdatePaused();
		SoundManagerObject::SetUpdatePaused(isPaused);
		if (isPaused) {
			SoundManager::Get().PauseAll();
		} else {
			SoundManager::Get().ResumeAll();
		}
		char buf[64];
		sprintf_s(buf, "[Debug] Sound: %s\n", isPaused ? "PAUSE" : "RESUME");
		OutputDebugStringA(buf);
	}
	prevF7 = curF7;

	// F8 : UI 描画 ON/OFF（UIManager管理UI＋InGameMenuを一括で止める）
	static bool prevF8 = false;
	const bool curF8 = (GetAsyncKeyState(VK_F8) & 0x8000) != 0;
	if (curF8 && !prevF8) {
		// drawMask_ の UI ビットを ^= でトグルする（描画 ON ↔ OFF の切り替え）。
		// updateMask_ ではなく drawMask_ を変えているため、
		// UI の Update（動き）はそのままで「画面に表示するかどうか」だけを制御できる。
		drawMask_ ^= UpdateGroup::UI;
		ApplyGroupMasks();
		Layout::SetDrawPaused(!Layout::IsDrawPaused());
		char buf[64];
		// & で UI ビットを取り出し、今 ON か OFF かをデバッグ表示する。
		const bool isOn = (drawMask_ & UpdateGroup::UI) != 0;
		sprintf_s(buf, "[Debug] UI Draw: %s\n", isOn ? "ON" : "OFF");
		OutputDebugStringA(buf);
	}
	prevF8 = curF8;
}
#endif
