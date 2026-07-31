#include "stdafx.h"
#include "src/Actor/BossSpawner.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/BossParamBuilder.h"
#include "src/Actor/NPCController.h"
#include "src/Actor/Player.h"
#include "src/Actor/PlayerController.h"
#include "src/CharacterDataBase.h"



BossSpawner::BossSpawner()
{

}

BossSpawner::~BossSpawner()
{
	// ボスを破棄
	DeleteGO(boss_);
	DeleteGO(bossController_);
}

void BossSpawner::Update()
{
	// コントローラの更新
	// NOTE: Update()は直接呼んでいるため、UpdateWrapper()と違いActivate/Deactivateが自動では反映されない。
	//       ここで明示的にIsActive()を見て、Deactivate中はAIの行動選択自体を止める。
	{
		if (playerController_) { playerController_->Update(); } // プレイヤーのコントローラーの更新
		if (bossController_ && bossController_->IsActive()) { bossController_->Update(); } // ボスのコントローラーの更新
	}


	if (boss_) { boss_->Update(); }
}


BossParam BossSpawner::CreateBossData(const std::string& key)
{
	// keyからJSONを読み、BossParamを組み立てる
	// (モデルパス・アニメーションの中身はここでは一切知らない。BossParamBuilderだけが知っている)
	return BossParamBuilder::Build(key);
}

void BossSpawner::SpawnBoss(bool isPlayerControl)
{
	// データベースからはステージ選択で選ばれたボスのkeyを取得
	const std::string stageKey = CharacterDataBase::Get().GetStageKey();

	// JSON読み込みがここで完了
	BossParam param = CreateBossData(stageKey);

	// ボスを作って、見た目や当たり判定のデータを渡す
	boss_ = NewGO<BossCharacter>(0, "boss");
	boss_->SetupParam(param);


	// ボスキャラクターがある場合
	if (boss_) 
	{
		// TODO : ボスの挙動確認ができるように、自分で操作可能にしたい
		// コントローラーの設定
		{
			// プレイヤー
			if (isPlayerControl)
			{
				playerController_ = NewGO<PlayerController>(0, "playerController");
			}

			// ボス
			else
			{
				bossController_ = NewGO<NPCController>(0, "bossController");
				bossController_->SetBossKey(stageKey); // 対戦するボスをセット
				bossController_->SetBossCharacter(boss_); // ボスの情報をセット
				bossController_->SetAttackTarget(attackTarget_); // 攻撃対象をセット
			}
		}
	}
}


void BossSpawner::SetUpdate(const bool flg)
{
	boss_->SetUpdate(flg);
}

void BossSpawner::SetControlEnabled(const bool flg)
{
	if (bossController_) {
		if (flg) {			bossController_->Activate(); // コントローラが動くようにする
		} else {
			bossController_->Deactivate(); // コントローラが動かないようにする
		}
	}

	if (boss_) {
		boss_->SetMoveStop(!flg);
		if (!flg) {
			boss_->ChangeState(BossStateID::Idle); // カットシーン中に待機アニメーションをさせる
			SetUpdate(true); // ボスが更新されるようにする
		}
	}
}