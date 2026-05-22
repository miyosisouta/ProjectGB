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
#include "src/Camera/CameraSteering.h"

#include "src/collision/GhostBodyManager.h"
#include "src/collision/CollisionHitManager.h"

#include "src/Stage/GrassBendManager.h"
#include "src/UI/Menu.h"
#include "src/UI/GameClearMenu.h"
#include "src/UI/GameOverMenu.h"
#include "src/UI/GameStartMenu.h"
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
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	// プレイヤー
	{
		player_           = NewGO<Player>(PRIORITY_PLAYER, "player");
		playerController_ = NewGO<PlayerController>(PRIORITY_PLAYER_CONTROLLER, "playerController");
		if (playerController_) 
		{
			playerController_->SetTarget(player_);
			playerController_->Deactivate();
		}
		if (player_) 
		{
			auto* stateMachine = player_->GetStateMachine();
			stateMachine->ResetInput();
			player_->SetMoveVelocity(Vector3::Zero);
		}
	}

	// ボス
	{
		BossType stageType = BossType::enGorilla;
		GameModeType mode  = GameModeType::enNormal;
		CharacterDataBase::Get().SetStageType(stageType);
		CharacterDataBase::Get().SetGameModeType(mode);

		boss_ = new BossSpawner();
		boss_->SetAttackTarger(player_);
		boss_->SpawnBoss();

		boss_->SetControlEnabled(false);
	}

	// キャラクター用設定
	{
		CharacterDataBase::Get().SetPlayerNormalAttack(NormalAttackType::enBite);
		CharacterDataBase::Get().SetPlayerAbility(AbilityType::enDefault);
		CharacterDataBase::Get().SetPlayerUtility(UtilityType::enAvoid);

		player_->CreateSkill(
			CharacterDataBase::Get().GetPlayerParam().nAttack,
			CharacterDataBase::Get().GetPlayerParam().ability,
			CharacterDataBase::Get().GetPlayerParam().utility
		);

		AttackObjectManager::CreateInstance();
	}

	// ステージ
	stage_  = NewGO<StageManagerObject>(PRIORITY_STAGE, "stage");

	// スカイキューブ
	{
		skyCube_ = NewGO<SkyCube>(PRIORITY_SKYCUBE, "skyCube");
		skyCube_->SetType(enSkyCubeType_Day);
		skyCube_->SetScale(param->skyCubeScale);
	}

	// カメラ
	{
		cameraSteering_ = std::make_unique<CameraSteering>();

		CameraSteering::Config initConfig;
		initConfig.distance       = param->cameraParam.distance;
		initConfig.height         = param->cameraParam.height;
		initConfig.rotationSpeedX = param->cameraParam.rotSpeed;
		initConfig.rotationSpeedY = param->cameraParam.rotSpeed;
		CameraData initData;
		initData.fov      = Math::DegToRad(param->cameraParam.fovy);
		initData.nearClip = param->cameraParam.nearClip;
		initData.farClip  = param->cameraParam.farClip;
		cameraSteering_->SetConfig(initConfig);
		cameraSteering_->SetTargetCharacter(player_);

		auto gameCamera = std::make_shared<GameCamera>();
		gameCamera->SetState(initData);
		gameCameraController_ = gameCamera;

		CameraOption initOption;
		initOption.sensitivity = param->cameraParam.sensitivity;
		initOption.distance    = param->cameraParam.distance;
		initOption.fovDeg      = param->cameraParam.fovy;
		SetCameraOption(initOption);
	}

	// 演出
	{
		cutSceneScheduler_ = std::make_unique<TaskSchedulerSystem>();
		SetupEntryBossCutScene();
	}

	// その他
	{
		gameTimer_.Init();
		gameTimer_.SetLimitTime(param->gameTimeParam.limitTime);
		gameTimer_.SetWarningTime(param->gameTimeParam.warningTime);

		uiManager_.InitMission();

		MissionManager::CreateInstance();
		MissionManager::Get().InitByBossType(CharacterDataBase::Get().GetStageType());
		GrassBendManager::Initialize();
	}
}


BattleManager::~BattleManager()
{
	// UIManager のデストラクタが Layout を全て解放する

	DeleteGO(player_);
	DeleteGO(playerController_);
	DeleteGO(stage_);
	DeleteGO(skyCube_);

	if (boss_) {
		delete boss_;
		boss_ = nullptr;
	}

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
			if (UpdateEntryBoss()) break;

			uiManager_.ReleaseCutSceneLayout();
			SetupStartCutScene();
			gameState_ = GameState::GameStart;
		}
		case GameState::GameStart:
		{
			auto* menu = static_cast<GameStartMenu*>(uiManager_.GetCutSceneMenu());
			if (menu && menu->IsCompletedStartCutSceen()) {
				uiManager_.ReleaseCutSceneLayout();
				uiManager_.InitTimer();

				playerController_->Activate();
				boss_->SetControlEnabled(true);
				boss_->SetUpdate(true);
				gameState_ = GameState::Playing;
			}
			break;
		}
		case GameState::Playing:
		{
			gameTimer_.Update();
			boss_->Update();
			GrassBendManager::Get().Update(g_gameTime->GetFrameDeltaTime());
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
			gameTimer_.Reset();
			g_ditherCBData.isEnable = DITHERING_ENABLE_FALSE_VALUE;
			break;
		}
	}

	uiManager_.Update();

	AttackObjectManager::Get().Update();
	GhostBodyManager::Get().Update();
	CollisionHitManager::Get().Update();
	KeyConfig::Get().Update();
}


void BattleManager::Render(RenderContext& rc)
{
	uiManager_.Render(rc);
	AttackObjectManager::Get().Render(rc);
}


void BattleManager::SetGroupMask(const uint32_t updateMask, const uint32_t drawMask)
{
	updateMask_ = updateMask;
	drawMask_   = drawMask;
	ApplyGroupMasks();
}


void BattleManager::ApplyGroupMasks()
{
	// --- Update 制御 ---
	if (player_)
		player_->SetUpdate((updateMask_ & UpdateGroup::Player) != 0);

	if (boss_)
		boss_->SetUpdate((updateMask_ & UpdateGroup::Boss) != 0);

	if (cameraSteering_)
		cameraSteering_->SetUpdate((updateMask_ & UpdateGroup::Camera) != 0);

	if (stage_)
		stage_->SetUpdate((updateMask_ & UpdateGroup::Stage) != 0);

	// --- UI の Update / Draw 制御 ---
	uiManager_.SetUpdateMask((updateMask_ & UpdateGroup::UI) ? UIManager::All : UIManager::None);
	uiManager_.SetDrawMask  ((drawMask_   & UpdateGroup::UI) ? UIManager::All : UIManager::None);
}


bool BattleManager::UpdateEntryBoss()
{
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	if (g_pad[0]->IsTrigger(enButtonA)) {
		isPlayingEntryBoss_ = false;
		CameraManager::Get().Unregister(GameCamera::ID());
		CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
		CameraManager::Get().SwitchCamera(gameCameraController_);
	}

	return isPlayingEntryBoss_;
}


bool BattleManager::UpdateResultClear()
{
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	auto* menu = dynamic_cast<GameClearMenu*>(uiManager_.GetCutSceneMenu());
	if (menu && menu->IsEnd()) {
		isPlayingResult_ = false;
	}

	return isPlayingResult_;
}


bool BattleManager::UpdateResultOver()
{
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	auto* menu = dynamic_cast<GameOverMenu*>(uiManager_.GetCutSceneMenu());
	if (menu && menu->IsEnd()) {
		isPlayingResult_ = false;
	}

	return isPlayingResult_;
}


void BattleManager::SetupEntryBossCutScene()
{
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	isPlayingEntryBoss_ = true;

	// カットシーン UI を生成
	uiManager_.SetupCutScene<MenuBase>("Assets/ui/layout/BossEntryCutScene.json");

	// 演出カメラ
	CameraData initData;
	initData.fov     = Math::DegToRad(param->cameraParam.fovy);
	initData.farClip = param->cameraParam.farClip;
	auto gameCamera  = std::make_shared<GameCamera>();
	gameCamera->SetState(initData);
	bossEntryCameraController_ = gameCamera;

	CameraManager::Get().Register(GameCamera::ID(), bossEntryCameraController_);
	CameraManager::Get().SwitchCamera(bossEntryCameraController_);

	// カットシーンスケジューラー
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

	cutSceneScheduler_->AddTimer(param->cutSceneParam.secondCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.secondCutCameraPos;
			cameraData.target   = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("gollira_nameC"))->isDraw = true;
		});

	cutSceneScheduler_->AddTimer(param->cutSceneParam.thirdCutTime, [this, param]()
		{
			CameraData cameraData;
			cameraData.position = param->cutSceneParam.thirdCutCameraPos;
			cameraData.target   = param->cutSceneParam.cutSceneTargetPos;
			bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

			auto* menu = uiManager_.GetCutSceneMenu();
			menu->GetUI<UIIcon>(Hash32("gollira_nameB"))->isDraw = true;
		});

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


void BattleManager::SetupStartCutScene()
{
	uiManager_.SetupCutScene<GameStartMenu>("Assets/ui/layout/GameStartMenu.json");
}


void BattleManager::SetupClearCutScene()
{
	isPlayingResult_ = true;
	cutSceneScheduler_->Reset();
	uiManager_.SetupCutScene<GameClearMenu>("Assets/ui/layout/GameClearMenu.json");
}


void BattleManager::SetupOverCutScene()
{
	isPlayingResult_ = true;
	cutSceneScheduler_->Reset();
	uiManager_.SetupCutScene<GameOverMenu>("Assets/ui/layout/GameOverMenu.json");
}


#ifdef K2_DEBUG
void BattleManager::UpdateDebugGroupInput()
{
	// F1〜F4 のトリガー検出（前フレームの押下状態を static で保持）
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
		const bool cur = (GetAsyncKeyState(table[i].vKey) & 0x8000) != 0;
		if (cur && !prev[i]) {
			updateMask_ ^= table[i].group;  // トグル
			changed = true;

			char buf[64];
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
		drawMask_ ^= UpdateGroup::UI;
		ApplyGroupMasks();
		Layout::SetDrawPaused(!Layout::IsDrawPaused());
		char buf[64];
		const bool isOn = (drawMask_ & UpdateGroup::UI) != 0;
		sprintf_s(buf, "[Debug] UI Draw: %s\n", isOn ? "ON" : "OFF");
		OutputDebugStringA(buf);
	}
	prevF8 = curF8;
}
#endif
