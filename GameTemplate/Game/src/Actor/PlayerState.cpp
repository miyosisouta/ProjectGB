#include "stdafx.h"
#include "PlayerState.h"
#include "Player.h"
#include "src/collision/GhostBody.h"


namespace
{
	const float WALK_BASE_SPEED = 100.0f;
	const float RUN_BASE_SPEED = 300.0f;
}


/*==================================================*/
/******************** 待機状態 **********************/
/*==================================================*/

void IdleState::Enter()
{
	// 待機アニメーション
	player_->PlayAnimation(StateID::Idle);
}

void IdleState::Update()
{
	// 移動速度はゼロに
	player_->SetMoveVelocity(Vector3::Zero);
}

void IdleState::Exit()
{
}


/*==================================================*/
/******************** 歩き状態 **********************/
/*==================================================*/

void WalkState::Enter()
{
	// 歩きアニメーショ
	player_->PlayAnimation(StateID::Walk);
}

void WalkState::Update()
{
	// 共通関数に歩く時の基本スピードを渡す
	Vector3 velocity = CalcMovementVelocity(WALK_BASE_SPEED);

	// 移動先をPlayerに渡す
	player_->SetMoveVelocity(velocity);
}

void WalkState::Exit()
{
}


/*==================================================*/
/* 走る状態 */
/*==================================================*/

void RunState::Enter()
{
	// 走るアニメーション
	player_->PlayAnimation(StateID::Run);
}

void RunState::Update()
{
	// 共通関数に歩く時の基本スピードを渡す
	Vector3 velocity = CalcMovementVelocity(RUN_BASE_SPEED);

	// 移動先をPlayerに渡す
	player_->SetMoveVelocity(velocity);
}

void RunState::Exit()
{
}


/*==================================================*/
/* 通常攻撃状態 */
/*==================================================*/

void NormalAttackState::Enter()
{
	// 初期化
	{
		isCancelable_ = false;
		isFinished_ = false;
	}


	// 通常攻撃アニメーション
	player_->PlayAnimation(StateID::NormalAttack);
	// 移動速度はゼロに
	player_->SetMoveVelocity(Vector3::Zero);


	// 攻撃コリジョンの生成・破棄
	{
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
	
		// スケジュール
		{
			// 1. ゴースト当たり判定を0.1秒後に1フレームのみ作成
			taskScheduler_->AddTimer(0.1f, [&]()
				{
					// ゴーストコリジョンを生成
					attackHitbox_ = std::make_unique<GhostBody>(); 
					attackHitbox_->CreateSphere(player_, Hash32("Player"), 50.0f, ghost::CollisionAttribute::Player, ghost::CollisionAttributeMask::Enemy);

					// 座標計算
					Vector3 playerPos = player_->transform_.position;				// プレイヤーの現在の座標を取得
					Quaternion playerRot = player_->GetStateMachine()->GetRotation(); 
					Vector3 forwardDir = player_->GetStateMachine()->GetDirection();// プレイヤーが最後に向いていた方向を取得
					float forwardOffset = 100.0f;									// 目の前にどれくらいズラすか
					float heightOffset = 50.0f;										// 高さの調整
					Vector3 targetPos = playerPos + (forwardDir * forwardOffset);	// 前方向の座標を決定
					targetPos.y += heightOffset;									// 高さを決定

					// コリジョンの座標を設定
					attackHitbox_->SetPosition(targetPos);
					// エフェクトのPRSを決める
					Quaternion hoge = playerRot;
					playerRot.AddRotationDegY(360.0f);
					EffectManager::Get().PlayEffect(enEffectKind_Wind_Blast, targetPos, playerRot,Vector3(2.0f,2.0f,2.0f));
					// 通常攻撃のSEを再生
					SoundManager::Get().PlaySE(enSoundKind_Player_NormalAttack);
				});
			// 2. 1フレーム生成後、削除
			taskScheduler_->AddTimer(0.1f, [&]()
				{
					attackHitbox_.reset(nullptr);
				},
				true);
			// 3. 0.3秒経過後回避が可能
			taskScheduler_->AddTimer(0.4f, [&]() {
				isCancelable_ = true;
				});

			// 4. 0.8秒後：攻撃（硬直）が完全に終わる！
			taskScheduler_->AddTimer(0.6f, [&]() {
				isFinished_ = true;
				});
		}
	}
}

void NormalAttackState::Update()
{
	if (attackHitbox_) {
		attackHitbox_->SetPosition(player_->transform_.position);

	}

	taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
}

void NormalAttackState::Exit()
{
	taskScheduler_.reset(nullptr);
	
}


/*==================================================*/
/* 死亡状態 */
/*==================================================*/

void DeadState::Enter()
{
	// 死亡アニメーション
	player_->PlayAnimation(StateID::Dead);
	// 移動速度はゼロに
	player_->SetMoveVelocity(Vector3::Zero);
}

void DeadState::Update()
{
	
}

void DeadState::Exit()
{
}



/*=========================================*/
/** ここからは共通処理 */
/*=========================================*/



Vector3 PlayerStateBase::CalcMovementVelocity(float speed)
{
	// ステートマシーンを取得
	auto* stateMachine = player_->GetStateMachine();

	// 左スティックの入力量を取得
	float stickLAmount = stateMachine->GetStickLAmount();

	// 入力がなければゼロベクトルを返す
	if (stickLAmount < 0.01f) { return Vector3::Zero; }

	// 方向を取得
	Vector3 dir = stateMachine->GetDirection();

	// スピードとフレームレート(DeltaTime)を掛けて返す
	return dir * speed * g_gameTime->GetFrameDeltaTime();
}
