#include "stdafx.h"
#include "DefaultAttack.h"
#include "src/Actor/Player.h"

void DefaultAttack::Enter(Character* p)
{
	// 初期化
	{
		isCancelable_ = false;
		isFinished_ = false;
	}


	// 通常攻撃アニメーション
	p->PlayAnimation(static_cast<int>(PlayerStateID::defaultAttack));
	// 移動速度はゼロに
	p->SetMoveVelocity(Vector3::Zero);


	// 攻撃コリジョンの生成・破棄
	{
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// スケジュール
		{
			// 1. ゴースト当たり判定を0.1秒後に1フレームのみ作成
			taskScheduler_->AddTimer(0.1f, [&, p]()
				{
					// ゴーストコリジョンを生成
					attackHitbox_ = std::make_unique<GhostBody>();
					attackHitbox_->CreateSphere(p, CharacterID::PlayerSkillAtkID(), 50.0f, ghost::CollisionAttribute::PlayerAtk, ghost::CollisionAttributeMask::PlayerAtk);

					// 座標計算
					p->transform_.UpdateTransform();
					Vector3 playerPos = p->transform_.position;				// プレイヤーの現在の座標を取得
					Quaternion playerRot = p->GetStateMachine()->GetRotation();
					Vector3 forwardDir = p->GetStateMachine()->GetDirection();// プレイヤーが最後に向いていた方向を取得
					float forwardOffset = 100.0f;									// 目の前にどれくらいズラすか
					float heightOffset = 50.0f;										// 高さの調整
					Vector3 targetPos = playerPos + (forwardDir * forwardOffset);	// 前方向の座標を決定
					targetPos.y += heightOffset;									// 高さを決定

					// コリジョンの座標を設定
					attackHitbox_->SetPosition(targetPos);
					// エフェクトのPRSを決める
					Quaternion hoge = playerRot;
					playerRot.AddRotationDegY(360.0f);
					EffectManager::Get().PlayEffect(enEffectKind_Wind_Blast_Strong, targetPos, playerRot, Vector3(9.0f, 9.0f, 9.0f));
					// 通常攻撃のSEを再生
					SoundManager::Get().PlaySE(enSoundKind_Player_Bite);
				});
			// 2. 1フレーム生成後、削除
			taskScheduler_->AddTimer(0.1f, [&]()
				{
					attackHitbox_.reset(nullptr);
				},
				true);
			// 3. 0.3秒経過後回避が可能
			taskScheduler_->AddTimer(0.5f, [&]() {
				isCancelable_ = true;
				});

			// 4. 0.8秒後：攻撃（硬直）が完全に終わる！
			taskScheduler_->AddTimer(0.8f, [&]() {
				isFinished_ = true;
				});
		}
	}
}

void DefaultAttack::Update(Character* p)
{
	if (attackHitbox_) {
		attackHitbox_->SetPosition(p->transform_.position);

	}

	taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
}

void DefaultAttack::Exit(Character* p)
{
	taskScheduler_.reset();
}
