/**
 * BattleManager.cpp
 * 
 * インゲーム管理
 * インゲームのゲームオブジェクトはこのクラスで管理する。
 * クラス間の情報伝達もこのクラスを介して行う。
 */


#include "stdafx.h"
#include "BattleManager.h"

#include "Src/Actor/Player.h"
#include "Src/Actor/PlayerController.h"
#include "src/Actor/BossSpawner.h"
#include "src/Actor/BossCharacter.h"
#include "src/Stage/Stage.h"

#include "src/Camera/CameraManager.h"
#include "src/Camera/CameraSteering.h"

#include "src/collision/GhostBodyManager.h"
#include "src/collision/CollisionHitManager.h"

#include "src/UI/Layout.h"
#include "src/UI/Menu.h"


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
	player_ = NewGO<Player>(0, "player");			//Playerの生成
	playerController_ = NewGO<PlayerController>(10, "playerController");	//PlayerControllerの生成
	playerController_->SetTarget(player_);		//PlayerControllerの操作対象をPlayerに設定

	boss_ = new BossSpawner(); // ボス生成用クラス
	boss_->SetAttackTarger(player_);
	boss_->SpawnBoss(); // ボスを作成

	stage_ = NewGO<Stage>(0, "stage");		//Stageの生成

	auto* skyCube = NewGO<SkyCube>(0,"skyCube");
	skyCube->SetType(enSkyCubeType_Day);
	skyCube->SetScale(SKYCUBE_SCALE);
	skyCube_ = skyCube;

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
		initData.farClip = CAMERA_FAR;
		cameraSteering_->SetConfig(initConfig);
		cameraSteering_->SetTargetCharacter(player_);

		auto gameCamera = std::make_shared<GameCamera>();
		gameCamera->SetState(initData);
		gameCameraController_ = gameCamera;
		//CameraManager::Get().Register(GameCamera::ID(), gameCameraController_);
		//CameraManager::Get().SwitchCamera(gameCameraController_);
	}

	// TODO : スキルの設定のテスト
	CharacterDataBase::Get().SetPlayerNormalAttack(NormalAttackType::enBite);
	CharacterDataBase::Get().SetPlayerAbility(AbilityType::enDefault);
	CharacterDataBase::Get().SetPlayerUtility(UtilityType::enAvoid);

	player_->CreateSkill(
		CharacterDataBase::Get().GetPlayerParam().nAttack,
		CharacterDataBase::Get().GetPlayerParam().ability,
		CharacterDataBase::Get().GetPlayerParam().utility
	);

	// レイアウト生成
	layout_ = new Layout();
	layout_->Initialize<MenuBase>("Assets/ui/layout/BossEntryCutScene.json");

	// ボス登場演出
	{
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
			entryBossScheduler_ = std::make_unique<TaskSchedulerSystem>();
			entryBossScheduler_->AddTimer(1.0f, [this]()
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

					SoundManager::Get().PlaySE(enSoundKind_Gorilla_SingleImpact);
				});
			entryBossScheduler_->AddTimer(2.0f, [this]()
				{
					CameraData cameraData;
					cameraData.position = Vector3(-600, 100.0f, 0.0f);
					cameraData.target = Vector3(-300.0f, 100.0f, 0.0f);
					bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

					auto* menu = layout_->GetMenu();
					auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameC"));
					icon->isDraw = true;

					SoundManager::Get().PlaySE(enSoundKind_Gorilla_SingleImpact);
				});
			entryBossScheduler_->AddTimer(3.0f, [this]()
				{
					CameraData cameraData;
					cameraData.position = Vector3(-300, 100.0f, 400.0f);
					cameraData.target = Vector3(-300.0f, 100.0f, 0.0f);
					bossEntryCameraController_->As<GameCamera>()->SetState(cameraData);

					auto* menu = layout_->GetMenu();
					auto* icon = menu->GetUI<UIIcon>(Hash32("gollira_nameB"));
					icon->isDraw = true;

					SoundManager::Get().PlaySE(enSoundKind_Gorilla_DoubleImpact);
				});
			// 終了
			entryBossScheduler_->AddTimer(7.0f, [this]()
				{
					isEndEntryBoss_ = false;
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
}


BattleManager::~BattleManager()
{
	if (layout_) {
		delete layout_;
		layout_ = nullptr;
	}
}


void BattleManager::Update()
{
	if (UpdateEntryBoss()) {
		layout_->Update();
		return;
	}

	if (layout_) {
		delete layout_;
		layout_ = nullptr;
	}

	boss_->Update();
	//gameCamera_->Update();		//GameCameraの更新

	auto gameCamera = gameCameraController_->As<GameCamera>();
	auto cameraData = gameCamera->GetCameraData();
	cameraSteering_->Update(cameraData, g_gameTime->GetFrameDeltaTime());
	gameCamera->SetState(cameraData);

	if (skyCube_) {
		Vector3 cameraPos = cameraData.position;
		skyCube_->SetPosition(cameraPos);
	}

	GhostBodyManager::Get().Update();
	CollisionHitManager::Get().Update();
}


void BattleManager::Render(RenderContext& rc)
{
	if (layout_) {
		layout_->Render(rc);
	}
	if (skyCube_) {
		skyCube_->Render(rc);
	}
}


bool BattleManager::UpdateEntryBoss()
{
	entryBossScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	return isEndEntryBoss_;
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