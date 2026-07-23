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
			// プレイヤーの値の取得
			p->transform_.UpdateTransform();
			Vector3    playerPos     = p->transform_.position;				 // プレイヤーの現在の座標を取得
			Quaternion playerRot     = p->GetStateMachine()->GetRotation();	 // プレイヤーの回転を取得
			Vector3    forwardDir    = p->GetStateMachine()->GetDirection(); // プレイヤーが向いている方向を取得

			// コリジョン・エフェクトのPRSを算出する（サイズ・座標・スケール）
			Vector3    collisionSize = Vector3(sp->fireMagicCollisionSize, sp->fireMagicCollisionSize, sp->fireMagicCollisionDepth); // コリジョンのサイズ
			float      forwardOffset = sp->fireMagicCollisionForward + sp->fireMagicCollisionDepth; // コリジョンを前方にズラす距離
			float      heightOffset  = sp->fireMagicCollisionHeight; // コリジョンの高さ調整
			Vector3    targetPos     = playerPos + (forwardDir * forwardOffset); // コリジョンの座標
			targetPos.y             += heightOffset; // 高さを調整
			Vector3    effectPos     = playerPos + (forwardDir * sp->fireMagicCollisionForward); // エフェクトの座標
			effectPos.y              = sp->fireMagicCollisionHeight; // エフェクトの高さ
			Vector3    effectScal    = collisionSize * sp->effectScaleFactor; // エフェクトのスケール（コリジョンサイズに倍率を掛ける）

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
