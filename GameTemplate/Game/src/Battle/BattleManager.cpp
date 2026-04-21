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


namespace
{
	// TODO: あとでパラメータを外部から読み込むようにする
	// カメラ近平面
	constexpr float CAMERA_NEAR = 1.0f;
	// カメラ遠平面
	constexpr float CAMERA_FAR = 50000.0f;
	// カメラの画角
	constexpr float CAMERA_FOVY = 60.0f;
	// カメラの高さ
	constexpr float CAMERA_HEIGHT = 200.0f;
	// カメラとプレイヤーの距離
	constexpr float CAMERA_DISTANCE = 600.0f;
	// カメラ回転速度
	constexpr float CAMERA_ROT_SPEED = 0.05f;

	// スカイキューブ
	constexpr float SKYCUBE_SCALE = 400.0f;
}


BattleManager* BattleManager::myInstance_ = nullptr; //初期化


BattleManager::BattleManager()
{
	// プレイヤー
	{
		player_ = NewGO<Player>(0, "player");			//Playerの生成
		playerController_ = NewGO<PlayerController>(10, "playerController");	//PlayerControllerの生成
		playerController_->SetTarget(player_);		//PlayerControllerの操作対象をPlayerに設定
	}
	

	// ボス
	{
		// test::ステージ選択画面がまだ作られていないのでここでボスデータを作る
		BossType stageType = BossType::enTurtle;
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
	stage_ = NewGO<StageManagerObject>(0, "stage");


	// スカイキューブ
	{
		skyCube_ = NewGO<SkyCube>(0, "skyCube");
		skyCube_->SetType(enSkyCubeType_Day);
		skyCube_->SetScale(SKYCUBE_SCALE);
	}
	

	// カメラ初期化
	{
		cameraSteering_ = std::make_unique<CameraSteering>();

		CameraSteering::Config initConfig;
		initConfig.distance = CAMERA_DISTANCE;
		initConfig.height = CAMERA_HEIGHT;
		initConfig.rotationSpeedX = CAMERA_ROT_SPEED;
		initConfig.rotationSpeedY = CAMERA_ROT_SPEED;
		CameraData initData;
		initData.fov = Math::DegToRad(CAMERA_FOVY);
		initData.nearClip = CAMERA_NEAR;
		initData.farClip = CAMERA_FAR;
		cameraSteering_->SetConfig(initConfig);
		cameraSteering_->SetTargetCharacter(player_);

		auto gameCamera = std::make_shared<GameCamera>();
		gameCamera->SetState(initData);
		gameCameraController_ = gameCamera;
		//CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
		//CameraManager::Get().SwitchCamera(gameCameraController_);
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

		// ミッションを作成
		MissionManager::CreateInstance();
		MissionManager::Get().InitByBossType(CharacterDataBase::Get().GetStageType());
	}
}


BattleManager::~BattleManager()
{
	ReleaseCutSceneLayout();

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
				gameState_ = GameState::Playing;
			}
			break;
		}
		case GameState::Playing:
		{
			gameTimer_.Update();
			boss_->Update();
			MissionManager::Get().Update();

			// カメラの更新
			auto gameCamera = gameCameraController_->As<GameCamera>();
			auto cameraData = gameCamera->GetCameraData();
			cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());
			gameCamera->SetState(cameraData);

			// スカイキューブをプレイヤーに追従させる
			skyCube_->SetPosition(player_->GetTransformPosition());

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

	
	AttackObjectManager::Get().Update();
	GhostBodyManager::Get().Update();
	CollisionHitManager::Get().Update();
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
	isPlayingEntryBoss_ = true;

	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<MenuBase>("Assets/ui/layout/BossEntryCutScene.json");

	// 演出カメラ
	// NOTE: GameCameraを使い回しているが必要に応じて専用のカメラコントローラーを作りたい
	CameraData initData;
	initData.fov = Math::DegToRad(CAMERA_FOVY);
	initData.farClip = CAMERA_FAR;
	auto gameCamera = std::make_shared<GameCamera>();
	gameCamera->SetState(initData);
	bossEntryCameraController_ = gameCamera;

	CameraManager::Get().Register(GameCamera::ID(), bossEntryCameraController_);
	CameraManager::Get().SwitchCamera(bossEntryCameraController_);

	// スケジューラーを作成
	{
		cutSceneScheduler_->AddTimer(1.0f, [this]()
			{
				CameraData cameraData;
				cameraData.position = Vector3(0, 100.0f, 0.0f);
				cameraData.target = Vector3(-300.0f, 100.0f, 0.0f);
				bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira"));
				icon->isDraw = true;
				icon = menu->GetUI<UIIcon>(Hash32("gollira_nameA"));
				icon->isDraw = true;
			});
		cutSceneScheduler_->AddTimer(2.0f, [this]()
			{
				CameraData cameraData;
				cameraData.position = Vector3(-600, 100.0f, 0.0f);
				cameraData.target = Vector3(-300.0f, 100.0f, 0.0f);
				bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameC"));
				icon->isDraw = true;
			});
		cutSceneScheduler_->AddTimer(3.0f, [this]()
			{
				CameraData cameraData;
				cameraData.position = Vector3(-300, 100.0f, 400.0f);
				cameraData.target = Vector3(-300.0f, 100.0f, 0.0f);
				bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

				auto* menu = layout_->GetMenu();
				auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameB"));
				icon->isDraw = true;

			});
		// 終了
		cutSceneScheduler_->AddTimer(7.0f, [this]()
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