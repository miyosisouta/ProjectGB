#include "stdafx.h"
#include "src/Actor/BossSpawner.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/NPCController.h"
#include "src/Actor/Player.h"
#include "src/Actor/PlayerController.h"

namespace
{
	const float MODE_ATTACK_MULTIPLIER = 1.5f; // 攻撃力の倍率
	const float MODE_HP_MULTIPLIER = 2.0f; // 体力の倍率
}


BossSpawner::BossSpawner()
{

}

BossSpawner::~BossSpawner()
{
	// ボスを破棄
	DeleteGO(boss_);
}

void BossSpawner::Update()
{
	// コントローラの更新
	{
		if (playerController_) { playerController_->Update(); } // プレイヤーのコントローラーの更新
		if (bossController_) { bossController_->Update(); } // ボスのコントローラーの更新
	}


	if (boss_) { boss_->Update(); }
}


BossParam BossSpawner::CreateBossData(BossType type, GameModeType mode)
{
	BossParam param;
	param.stageType_ = type;
	param.mode_ = mode;

	// TODO　: ここでjsonファイルを読み込むつもり


	std::string modelpath = "Assets/Objects/Enemy/";
	// ボスの種類を決める
	switch (type)
	{
	case BossType::enGorilla:
	{
		// モデルtkm
		std::string path = modelpath + "Gorilla/Model/Gorilla.tkm";
		param.modelPath_ = path;

		// アニメ登録を自動化
		auto AddAnim = [&](BossAnimID id, const std::string& fileName, bool loop)
			{
				// ここでパスを結合して、paramに直接突っ込む！
				param.anims[id].filePath = modelpath + "Gorilla/Animation/" + fileName;
				param.anims[id].isLoop = loop;
			};

		// アニメーションの追加
		AddAnim(enAnimIdle,"IdleA.tka",true);
		AddAnim(enAnimRun,"Run.tka",true);
		AddAnim(enAnimJump,"Jump.tka",false);
		AddAnim(enAnimAttack,"Attack.tka",false);
		AddAnim(enAnimHit,"Hit.tka",false);
		AddAnim(enAnimDeath,"Death.tka",false);
		AddAnim(enAnimSpin, "Spin.tka", true);
		AddAnim(enAnimClicked, "Clicked.tka", false);
		break;
	}
	default:
		break;
	}


	// モード
	switch (mode)
	{
	case GameModeType::enHighAttack:
	{
		param.attack_ = static_cast<int>(param.attack_ * MODE_ATTACK_MULTIPLIER);
		break;
	}
	case GameModeType::enTimeAttack:
	{
		param.maxHp_ = static_cast<int>(param.maxHp_ * MODE_HP_MULTIPLIER);

		// TODO : 仮で最大体力を設定
		param.maxHp_ = 5.0f;
		break;
	}
	default:
		break;
	}
	return param;
}

void BossSpawner::SpawnBoss(bool isPlayerControl)
{
	// データベースからはステージ選択で選ばれたタイプを取得
	//BossType stageType = CharacterDataBase::Get().GetStageType();
	//GameModeType mode = CharacterDataBase::Get().GetGameModeType();

	BossType stageType = BossType::enGorilla;
	GameModeType mode = GameModeType::enNormal;


	// JSON読み込み ＋ モード補正 がここで完了
	BossParam param = CreateBossData(stageType, mode);

	// ボスを作って、見た目や当たり判定のデータを渡す
	boss_ = NewGO<BossCharacter>(0, "boss");
	boss_->SetupParam(param);


	// ボスキャラクターがある場合
	if (boss_) 
	{
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
				bossController_->SetBossType(stageType); // 対戦するボスをセット
				bossController_->SetBossCharacter(boss_); // ボスの情報をセット
				bossController_->SetAttackTarget(attackTarget_); // 攻撃対象をセット
			}
		}
	}
}
