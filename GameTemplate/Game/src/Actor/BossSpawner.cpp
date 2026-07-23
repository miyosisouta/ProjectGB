#include "stdafx.h"
#include "src/Actor/BossSpawner.h"
#include "src/Actor/BossCharacter.h"
#include "src/Actor/NPCController.h"
#include "src/Actor/Player.h"
#include "src/Actor/PlayerController.h"



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


BossParam BossSpawner::CreateBossData(BossType type, GameModeType mode)
{
	BossParam param;
	param.stageType_ = type;
	param.mode_ = mode;

	// 共通のモデルパスを定義
	std::string modelpath = "Assets/Objects/Enemy/";


	// ボスの種類を決める
	switch (type)
	{
	case BossType::enGorilla: /**************** ゴリラ *************************/
	{
		// モデルtkm
		std::string path = modelpath + "Gorilla/Model/Gorilla.tkm";
		param.characterKey_ = "Gorilla";
		param.modelPath_ = path;
		param.modelAxis_ = EnModelUpAxis::enModelUpAxisZ;

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
		AddAnim(enAnimClickedWindUp, "Clicked_wideUp.tka", true);
		AddAnim(enAnimJumpImpact, "Jump_Impact.tka", false);
		break;
	}
	case BossType::enTurtle:/**************** カメ *************************/
	{
		// モデルtkm
		std::string path = modelpath + "Turtle/Model/Turtle.tkm";
		param.characterKey_ = "Turtle";
		param.modelPath_ = path;
		param.modelAxis_ = EnModelUpAxis::enModelUpAxisZ;

		// アニメ登録を自動化
		auto AddAnim = [&](BossAnimID id, const std::string& fileName, bool loop)
			{
				// ここでパスを結合して、paramに直接突っ込む！
				param.anims[id].filePath = modelpath + "Turtle/Animation/" + fileName;
				param.anims[id].isLoop = loop;
			};

		// アニメーションの追加
		AddAnim(enAnimIdle, "IdleA.tka", true);
		AddAnim(enAnimRun, "Run.tka", true);
		AddAnim(enAnimJump, "Jump.tka", false);
		AddAnim(enAnimAttack, "Attack.tka", false);
		AddAnim(enAnimHit, "Hit.tka", false);
		AddAnim(enAnimDeath, "Death.tka", false);
		AddAnim(enAnimSpin, "Spin.tka", true);
		AddAnim(enAnimClicked, "Clicked.tka", false);
		AddAnim(enAnimAntic, "Antic.tka", true);
		AddAnim(enAnimJumpImpact, "Jump_Impact.tka", false);
		break;
	}
	default:
		break;
	}


	// モード
	const auto* modeParam = ParameterManager::Get().GetBossSpawnerParam();
	switch (mode)
	{
	case GameModeType::enHighAttack: /* 攻撃力が高いモード */
	{
		param.attack_ = static_cast<int>(param.attack_ * modeParam->modeAttackMultiplier);
		break;
	}
	case GameModeType::enTimeAttack: /* 防御力が高いモード */
	{
		param.maxHp_ = static_cast<int>(param.maxHp_ * modeParam->modeHpMultiplier);

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
	BossType stageType = CharacterDataBase::Get().GetStageType();
	GameModeType mode = CharacterDataBase::Get().GetGameModeType();


	// JSON読み込み ＋ モード補正 がここで完了
	BossParam param = CreateBossData(stageType, mode);

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
				bossController_->SetBossType(stageType); // 対戦するボスをセット
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