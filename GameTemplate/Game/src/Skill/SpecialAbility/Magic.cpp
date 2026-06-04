#include "stdafx.h"
#include "Magic.h"
#include "src/Actor/Player.h"




/**==================================*/
/** 魔法関連の基底クラス */
/**==================================*/

MagicBase::MagicBase()
{
}

MagicBase::~MagicBase()
{
}


/**==================================*/
/** 火魔法クラス */
/**==================================*/

FireMagic::FireMagic()
{
}

FireMagic::~FireMagic()
{
}

void FireMagic::Enter(Character* p)
{
	// 初期化
	{
		isCancelable_ = false;
		isFinished_ = false;
	}


	// 通常攻撃アニメーション
	p->PlayAnimation(static_cast<int>(PlayerStateID::FireMagic));
	// 移動速度はゼロに
	p->SetMoveVelocity(Vector3::Zero);


	// 攻撃コリジョンの生成・破棄
	{
		// タスクシステムを作成
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

		// 設置モーション終了でステートを解放
		const auto* sp = ParameterManager::Get().GetSkillParam("FireMagic");
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		taskScheduler_->AddTimer(sp->fireMagicAttackStartTime, [&, p, sp]() {

			// 必要な情報を取得・計算
			p->transform_.UpdateTransform(); // プレイヤーのトランスフォームを更新
			Vector3 playerPos = p->transform_.position; // プレイヤーの座標を取得
			Quaternion playerRot = p->GetStateMachine()->GetRotation();
			Vector3 forwardDir = p->GetStateMachine()->GetDirection();// プレイヤーが最後に向いていた方向を取得
			float forwardOffset = sp->fireMagicCollisionForward + sp->fireMagicCollisionDepth; // 目の前にどれくらいズラすか
			float heightOffset  = sp->fireMagicCollisionHeight; // 高さの調整
			Vector3 collisionSize = Vector3(sp->fireMagicCollisionSize, sp->fireMagicCollisionSize, sp->fireMagicCollisionDepth);
			Vector3 targetPos = playerPos + (forwardDir * forwardOffset); // 前方向の座標を決定
			targetPos.y += heightOffset; // 高さを決定
			Vector3 effectPos = playerPos + (forwardDir * sp->fireMagicCollisionForward);
			effectPos.y = sp->fireMagicCollisionHeight;
			Vector3 effectScal = collisionSize * sp->effectScaleFactor;

			// コリジョンの作成
			attackHitbox_ = std::make_unique<GhostBody>();

			attackHitbox_->CreateBox(
				p, 
				CharacterID::PlayerNormalAtkID(), 
				collisionSize,
				ghost::CollisionAttribute::PlayerAtk, 
				ghost::CollisionAttributeMask::PlayerAtk
			);
			// コリジョンの座標を設定
			attackHitbox_->SetPosition(targetPos);
			attackHitbox_->SetRotation(playerRot);

			// エフェクトのPRSを決める
			fireEffectHandle_ = EffectManager::Get().PlayEffect(enEffectKind_Fire_Bress, effectPos, playerRot, effectScal);
			
			// 通常攻撃のSEを再生
			SoundManager::Get().PlaySE(enSoundKind_Player_FireMagic);
			});

		taskScheduler_->AddTimer(sp->fireMagicAttackEndTime, [&, p]() {
			isFinished_ = true;
			});
	}
}

void FireMagic::Update(Character* p)
{
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void FireMagic::Exit(Character* p)
{
	EffectManager::Get().StopEffect(fireEffectHandle_);
	fireEffectHandle_ = INVALID_EFFECT_HANDLE;
	attackHitbox_.reset();
}
