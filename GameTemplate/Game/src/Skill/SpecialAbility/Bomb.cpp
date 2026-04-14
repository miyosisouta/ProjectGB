#include "stdafx.h"
#include "Bomb.h"
#include "src/Actor/Player.h"
#include "src/Actor/ActorStatus.h"
#include "src/Actor/AttackObjectManager.h"

/**==================================*/
/** 爆弾の基底クラス */
/**==================================*/

BombBase::BombBase()
{
}

BombBase::~BombBase()
{
}



/**==================================*/
/** 地雷クラス */
/**==================================*/

Landmine::Landmine()
{
}

Landmine::~Landmine()
{
}

void Landmine::Enter(Character* p)
{
	// 初期化
	{
		isCancelable_ = false;
		isFinished_ = false;
	}


	// 通常攻撃アニメーション
	p->PlayAnimation(static_cast<int>(PlayerStateID::Landmine));
	// 移動速度はゼロに
	p->SetMoveVelocity(Vector3::Zero);


	// 必要な情報を取得・計算
	p->transform_.UpdateTransform();			// プレイヤーのトランスフォームを更新
	Vector3 playerPos = p->transform_.position; // プレイヤーの座標を取得

	auto status = p->GetStatus()->As<PlayerStatus>();					// プレイヤーのステータスを取得
	float motionValue = status->GetSkillMotionValues("SpecialAttack");	// プレイヤーステータスの中のスキルの威力を取得


	// 攻撃コリジョンの生成・破棄
	{
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// 設置位置を取得
		p->transform_.UpdateTransform();
		Vector3 pos = p->transform_.position;

		// Managerに地雷の生成を依頼するだけ
		AttackObjectManager::Get().CreateLandmine(p, pos, motionValue);

		// 設置モーション終了でステートを解放
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		taskScheduler_->AddTimer(0.5f, [&]() {
			isFinished_ = true;
			});
	}
}

void Landmine::Update(Character* p)
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void Landmine::Exit(Character* p)
{
	taskScheduler_.reset();
}


