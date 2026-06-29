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

	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	const auto* sp = ParameterManager::Get().GetSkillParam("FireMagic");

	// コリジョン・エフェクト・SEを生成して出しっぱなし
	// 多段ヒットは CollisionHitManager::OnCollisionStay 側で管理する
	taskScheduler_->AddTimer(sp->fireMagicAttackStartTime, [this, p, sp]()
		{
			p->transform_.UpdateTransform();
			Vector3    playerPos     = p->transform_.position;
			Quaternion playerRot     = p->GetStateMachine()->GetRotation();
			Vector3    forwardDir    = p->GetStateMachine()->GetDirection();
			Vector3    collisionSize = Vector3(sp->fireMagicCollisionSize, sp->fireMagicCollisionSize, sp->fireMagicCollisionDepth);
			float      forwardOffset = sp->fireMagicCollisionForward + sp->fireMagicCollisionDepth;
			float      heightOffset  = sp->fireMagicCollisionHeight;
			Vector3    targetPos     = playerPos + (forwardDir * forwardOffset);
			targetPos.y             += heightOffset;
			Vector3    effectPos     = playerPos + (forwardDir * sp->fireMagicCollisionForward);
			effectPos.y              = sp->fireMagicCollisionHeight;
			Vector3    effectScal    = collisionSize * sp->effectScaleFactor;

			attackHitbox_ = std::make_unique<GhostBody>();
			attackHitbox_->CreateBox(
				p,
				CharacterID::PlayerSkillAtkID(),
				collisionSize,
				ghost::CollisionAttribute::PlayerAtk,
				ghost::CollisionAttributeMask::PlayerAtk
			);
			attackHitbox_->SetPosition(targetPos);
			attackHitbox_->SetRotation(playerRot);

			fireEffectHandle_ = EffectManager::Get().PlayEffect(enEffectKind_Fire_Bress, effectPos, playerRot, effectScal);
			SoundManager::Get().PlaySE(enSoundKind_Player_FireMagic);
		});

	// 全ヒット終了後にステートを解放
	taskScheduler_->AddTimer(sp->fireMagicAttackEndTime, [this]()
		{
			isFinished_ = true;
		});
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
