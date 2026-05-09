/**
 * BattleManager.cpp
 * 
 * インゲーム管理
 * インゲームのゲームオブジェクトはこのクラスで管理する。
 * クラス間の情報伝達もこのクラスを介して行う。
 */


#include "stdafx.h"
#include "BattleManager.h"
#include "MissionManager.h"

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

#include "src/UI/Layout.h"
#include "src/UI/Menu.h"
#include "src/UI/GameClearMenu.h"
#include "src/UI/GameOverMenu.h"
#include "src/UI/GameStartMenu.h"
#include "src/UI/MissionMenu.h"
#include "src/UI/TimerMenu.h"
#include "../../k2Engine/graphics/DitherCBData.h"



namespace
{
	/* 優先度 */
	constexpr uint8_t PRIORITY_PLAYER = 0;				// プレイヤー
	constexpr uint8_t PRIORITY_PLAYER_CONTROLLER = 10;	// プレイヤーコントローラー
	constexpr uint8_t PRIORITY_STAGE = 0;				// ステージ
	constexpr uint8_t PRIORITY_SKYCUBE = 0;				// スカイキューブ

	/* ディザリング */
	constexpr float DITHERING_ENABLE_TRUE_VALUE = 1.0f; // ディザリング有効な値
	constexpr float DITHERING_ENABLE_FALSE_VALUE = 0.0f; // ディザリング無効な値

}

BattleManager* BattleManager::myInstance_ = nullptr; //初期化

BattleManager::BattleManager()
{
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);
	// プレイヤー
	{
		player_ = NewGO<Player>(PRIORITY_PLAYER, "player");			//Playerの生成
		playerController_ = NewGO<PlayerController>(PRIORITY_PLAYER_CONTROLLER, "playerController");	//PlayerControllerの生成
		playerController_->SetTarget(player_);		//PlayerControllerの操作対象をPlayerに設定
	}
	

	// ボス
	{
		// test::ステージ選択画面がまだ作られていないのでここでボスデータを作る
		BossType stageType = BossType::enGorilla;
		GameModeType mode = GameModeType::enNormal;
		CharacterDataBase::Get().SetStageType(stageType);
		CharacterDataBase::Get().SetGameModeType(mode);

		boss_ = new BossSpawner(); // ボス生成用クラス
		boss_->SetAttackTarger(player_); // ボスの攻撃対象を設定
		boss_->SpawnBoss(); // ボスを作成
	}
	

	// キャラクター用設定
	{
		// TODO : スキルの設定のテスト
		CharacterDataBase::Get().SetPlayerNormalAttack(NormalAttackType::enBite);
		CharacterDataBase::Get().SetPlayerAbility(AbilityType::enLandmine);
		CharacterDataBase::Get().SetPlayerUtility(UtilityType::enAvoid);

		// TODO : スキルの作成のテスト
		player_->CreateSkill(
			CharacterDataBase::Get().GetPlayerParam().nAttack,
			CharacterDataBase::Get().GetPlayerParam().ability,
			CharacterDataBase::Get().GetPlayerParam().utility
		);

		// 攻撃用オブジェクト管理クラスの作成
		AttackObjectManager::CreateInstance();
	}


	// ステージの作成
	stage_ = NewGO<StageManagerObject>(PRIORITY_STAGE, "stage");


	// スカイキューブ
	{
		skyCube_ = NewGO<SkyCube>(PRIORITY_SKYCUBE, "skyCube");
		skyCube_->SetType(enSkyCubeType_Day);
		skyCube_->SetScale(param->skyCubeScale);
	}
	

	// カメラ初期化
	{
		cameraSteering_ = std::make_unique<CameraSteering>();

		CameraSteering::Config initConfig;
		initConfig.distance = param->cameraParam.distance;
		initConfig.height = param->cameraParam.height;
		initConfig.rotationSpeedX = param->cameraParam.rotSpeed;
		initConfig.rotationSpeedY = param->cameraParam.rotSpeed;
		CameraData initData;
		initData.fov = Math::DegToRad(param->cameraParam.fovy);
		initData.nearClip = param->cameraParam.nearClip;
		initData.farClip = param->cameraParam.farClip;
		cameraSteering_->SetConfig(initConfig);
		cameraSteering_->SetTargetCharacter(player_);

		auto gameCamera = std::make_shared<GameCamera>();
		gameCamera->SetState(initData);
		gameCameraController_ = gameCamera;
		//CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
		//CameraManager::Get().SwitchCamera(gameCameraController_);

		CameraOption initOption;
		initOption.sensitivity = param->cameraParam.sensitivity;
		initOption.distance = param->cameraParam.distance;
		initOption.fovDeg = param->cameraParam.fovy;
		SetCameraOption(initOption);
	}

	// 演出
	{
		// 演出用カットシーンスケジューラーの作成
		cutSceneScheduler_ = std::make_unique<TaskSchedulerSystem>();
		// ボス登場演出
		SetupEntryBossCutScene();
	}


	// その他設定
	{
		// ミッション
		missionLayout_ = new Layout;
		missionLayout_->Initialize<MissionMenu>("Assets/ui/layout/MissionMenu.json");

		// ゲームタイマーを起動
		gameTimer_.Init();
		gameTimer_.SetLimitTime(param->gameTimeParam.limitTime);
		gameTimer_.SetWarningTime(param->gameTimeParam.warningTime);

		// ミッションを作成
		MissionManager::CreateInstance();
		MissionManager::Get().InitByBossType(CharacterDataBase::Get().GetStageType());
	}
}


BattleManager::~BattleManager()
{
	// レイアウトの削除
	{
		ReleaseCutSceneLayout();
		if (startLayout_) {
			delete startLayout_;
			startLayout_ = nullptr;
		}
		if (missionLayout_) {
			delete missionLayout_;
			missionLayout_ = nullptr;
		}
		if (timerLayout_) {
			delete timerLayout_;
			timerLayout_ = nullptr;
		}
	}
	

	DeleteGO(player_);
	DeleteGO(playerController_);
	DeleteGO(stage_);
	DeleteGO(skyCube_);
	if (boss_) {
		delete boss_;
		boss_ = nullptr;
	}

	MissionManager::Get().DestroyInstance();
}


void BattleManager::Update()
{
	// 状態によって処理を変更
	switch (gameState_)
	{
		case GameState::Entry:
		{
			if (UpdateEntryBoss()) {
				break;
			}
			ReleaseCutSceneLayout();
			// カメラ演出終わったので、バトル開始の演出をする
			SetupStartCutScene();
			gameState_ = GameState::GameStart;
		}
		case GameState::GameStart:
		{
			auto* menu = static_cast<GameStartMenu*>(startLayout_->GetMenu());
			if (menu->IsCompletedStartCutSceen()) {
				if (startLayout_) {
					delete startLayout_;
					startLayout_ = nullptr;
				}

				// ゲームタイマーのUIを作る
				timerLayout_ = new Layout;
				timerLayout_->Initialize<TimerMenu>("Assets/ui/layout/TimerMenu.json");

				gameState_ = GameState::Playing;
			}
			break;
		}
		case GameState::Playing:
		{
			gameTimer_.Update();
			boss_->Update();
			MissionManager::Get().Update();
			
			// ディザリングの設定
			g_ditherCBData.isEnable = DITHERING_ENAVLE_TRUE_VALUE;
			g_ditherCBData.cameraWorldPos = CameraManager::Get().GetCurrentCameraData().position;
			g_ditherCBData.playerWorldPos = player_->GetTransformPosition();

			// カメラの更新
			auto gameCamera = gameCameraController_->As<GameCamera>();
			auto cameraData = gameCamera->GetCameraData();
			cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(cameraData);

			// スカイキューブをプレイヤーに追従させる
			skyCube_->SetPosition(player_->GetTransformPosition());

			// 時間をUIに渡す
			{
				TimerMenu* menu = static_cast<TimerMenu*>(timerLayout_->GetMenu());
				menu->SetRate(gameTimer_.GetRate());

				menu->SetMinutes(gameTimer_.GetRemainMinutes());
				menu->SetSeconds(gameTimer_.GetRemainSeconds());

				menu->SetRequestFlashing(gameTimer_.IsWarningFrame());
			}
			
			// BossのHPが0になったらクリア演出へ
			auto* boss = FindGO<BossCharacter>("boss");
			if (boss && boss->GetStatus()) {
				if (boss->GetStatus()->IsDead()) {
					MissionManager::Get().NotifyBossDefeated(); // ボスを倒したら、通知を飛ばす
					SetupClearCutScene();
					gameState_ = GameState::ResultClear;
				}
			}

			if (player_->GetStatus()->IsDead() || gameTimer_.IsTimeUp())
			{
				SetupOverCutScene();
				gameState_ = GameState::ResultOver;
			}
			break;
		}
		case GameState::ResultClear:
		{
			g_ditherCBData.isEnable = DITHERING_ENAVLE_FALSE_VALUE;
			if (!UpdateResultClear()) {
				gameState_ = GameState::Shutdown;
			}
			break;
		}
		case GameState::ResultOver:
		{
			if (!UpdateResultOver()) {
				gameState_ = GameState::Shutdown;
			}
			break;
			
		}
		case GameState::Shutdown:
		{
			gameTimer_.Reset(); // ゲームタイマーを初期化
			break;
		}
	}

	if (layout_) {
		layout_->Update();
	}
	if (startLayout_) {
		startLayout_->Update();
	}
	if (missionLayout_) {
		missionLayout_->Update();
	}
	if (timerLayout_) {
		timerLayout_->Update();
	}

	
	AttackObjectManager::Get().Update();
	GhostBodyManager::Get().Update();
	CollisionHitManager::Get().Update();
	KeyConfig::Get().Update();
}


void BattleManager::Render(RenderContext& rc)
{
	if (layout_) {
		layout_->Render(rc);
	}
	if (startLayout_) {
		startLayout_->Render(rc);
	}
	if (missionLayout_) {
		missionLayout_->Render(rc);
	}
	if (timerLayout_) {
		timerLayout_->Render(rc);
	}
	AttackObjectManager::Get().Render(rc);
}


bool BattleManager::UpdateEntryBoss()
{
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());

	if (g_pad[0]->IsTrigger(enButtonA))
	{
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

	auto* menu = dynamic_cast<GameClearMenu*>(layout_->GetMenu());
	if (menu) {
		// 演出終了したか
		if (menu->IsEnd()) {
			isPlayingResult_ = false;
		}
	}

	return isPlayingResult_;
}


bool BattleManager::UpdateResultOver()
{
	cutSceneScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	auto* menu = dynamic_cast<GameOverMenu*>(layout_->GetMenu());
	if (menu) {
		// 演出終了したか
		if (menu->IsEnd()) {
			isPlayingResult_ = false;
		}
	}
	return isPlayingResult_;
}


void BattleManager::SetupEntryBossCutScene()
{
	const auto* param = ParameterManager::Get().GetParameter<MasterBattleCommonParameter>(0);

	isPlayingEntryBoss_ = true;

	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<MenuBase>("Assets/ui/layout/BossEntryCutScene.json");

	// 演出カメラ
	// NOTE: GameCameraを使い回しているが必要に応じて専用のカメラコントローラーを作りたい
	CameraData initData;
	initData.fov = Math::DegToRad(param->cameraParam.fovy);
	initData.farClip = param->cameraParam.farClip;
	auto gameCamera = std::make_shared<GameCamera>();
	gameCamera->SetState(initData);
	bossEntryCameraController_ = gameCamera;

	CameraManager::Get().Register(GameCamera::ID(), bossEntryCameraController_);
	CameraManager::Get().SwitchCamera(bossEntryCameraController_);

	// スケジューラーを作成
	{
		cutSceneScheduler_->AddTimer(param->cutSceneParam.firstCutTime, [this,param]()
			{
				CameraData cameraData;
				cameraData.position = param->cutSceneParam.firstCutCameraPos;
				cameraData.target = param->cutSceneParam.cutSceneTargetPos;
				bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira"));
				icon->isDraw = true;
				icon = menu->GetUI<UIIcon>(Hash32("gollira_nameA"));
				icon->isDraw = true;
			});
		cutSceneScheduler_->AddTimer(param->cutSceneParam.secondCutTime, [this, param]()
			{
				CameraData cameraData;
				cameraData.position = param->cutSceneParam.secondCutCameraPos;
				cameraData.target = param->cutSceneParam.cutSceneTargetPos;
				bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameC"));
				icon->isDraw = true;
			});
		cutSceneScheduler_->AddTimer(param->cutSceneParam.thirdCutTime, [this, param]()
			{
				CameraData cameraData;
				cameraData.position = param->cutSceneParam.thirdCutCameraPos;
				cameraData.target = param->cutSceneParam.cutSceneTargetPos;
				bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameB"));
				icon->isDraw = true;

			});
		// 終了
		cutSceneScheduler_->AddTimer(param->cutSceneParam.endCutTime, [this]()
			{
				isPlayingEntryBoss_ = false;
				CameraManager::Get().Unregister(GameCamera::ID());
				CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
				CameraManager::Get().SwitchCamera(gameCameraController_);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameB"));
				icon->isDraw = false;
				icon = menu->GetUI<UIIcon>(Hash32("gollira"));
				icon->isDraw = false;
			});
	}
}


void BattleManager::SetupStartCutScene()
{
	startLayout_ = new Layout;
	startLayout_->Initialize<GameStartMenu>("Assets/ui/layout/GameStartMenu.json");

}


void BattleManager::SetupClearCutScene()
{
	isPlayingResult_ = true;

	cutSceneScheduler_->Reset();

	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<GameClearMenu>("Assets/ui/layout/GameClearMenu.json");

	// NOTE: GameClearは GameClearMenu 側でUI演出の処理を書いている
}


void BattleManager::SetupOverCutScene()
{
	isPlayingResult_ = true;

	cutSceneScheduler_->Reset();

	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<GameOverMenu>("Assets/ui/layout/GameOverMenu.json");

	// NOTE: 必要であればUIアニメーション等を追加する
	//cutSceneScheduler_->AddTimer(3.0f, [this]()
	//	{
	//		// TODO : ゲームオーバー後の処理（タイトルに戻るなど）
	//		isPlayingResult_ = false;
	//	});
}


void BattleManager::ReleaseCutSceneLayout()
{
	if (layout_) {
		delete layout_;
		layout_ = nullptr;
	}
}


void BattleManager::SetActiveTarget(const uint32_t target)
{
	// 更新対象の切り替え
	// UIのみ
	if (target == UpdateGroup::UI) {
		player_->SetUpdate(false);
		boss_->SetUpdate(false);
		cameraSteering_.get()->SetUpdate(false);
	}

	// すべて
	else if (target == UpdateGroup::All) {
		player_->SetUpdate(true);
		boss_->SetUpdate(true);
		cameraSteering_.get()->SetUpdate(true);
	}
}