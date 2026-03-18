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
#include "src/Stage/Stage.h"
#include "src/Camera/GameCamera.h"


BattleManager* BattleManager::myInstance_ = nullptr; //初期化


BattleManager::BattleManager()
{
	player_ = NewGO<Player>(0, "player");			//Playerの生成
	playerController_ = NewGO<PlayerController>(10, "playerController");	//PlayerControllerの生成
	playerController_->SetTarget(player_);		//PlayerControllerの操作対象をPlayerに設定
	stage_ = NewGO<Stage>(0, "stage");		//Stageの生成
	gameCamera_ = new GameCamera();		//GameCameraの生成

	// TODO : スキルの設定のテスト
	CharacterDataBase::Get().SetPlayerNormalAttack(NormalAttackType::enBite);
	CharacterDataBase::Get().SetPlayerAbility(AbilityType::enDefault);
	CharacterDataBase::Get().SetPlayerUtility(UtilityType::enNone);

	player_->CreateSkill(
		CharacterDataBase::Get().GetPlayerParam().nAttack,
		CharacterDataBase::Get().GetPlayerParam().ability,
		CharacterDataBase::Get().GetPlayerParam().utility
	);
}


BattleManager::~BattleManager()
{
}


void BattleManager::Update()
{
	gameCamera_->Update();		//GameCameraの更新
}