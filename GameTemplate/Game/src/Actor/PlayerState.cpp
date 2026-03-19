#include "stdafx.h"
#include "PlayerState.h"
#include "Player.h"
#include "ActorStatus.h"
#include "src/Skill/NormalAttack/NormalAtatck.h"
#include "src/Skill/SpecialAbility/AbilityBase.h"
#include "src/Skill/Utility/Utility.h"
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
	player_->PlayAnimation(static_cast<int>(StateID::Idle));
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
	player_->PlayAnimation(static_cast<int>(StateID::Walk));
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
	player_->PlayAnimation(static_cast<int>(StateID::Run));
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
	// playerの通常攻撃タイプを取得
	currentSkill_ = player_->GetNormalAttackSkill();

	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Enter(player_);
	}
}

void NormalAttackState::Update()
{
	if (currentSkill_) { currentSkill_->Update(player_); }
}

void NormalAttackState::Exit()
{
	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Exit(player_);
	}
	// 借りていたポインタを空にする（deleteはされないので安全）
	currentSkill_ = nullptr;
}

bool NormalAttackState::IsFinished() const
{
	if (currentSkill_ && currentSkill_->IsFinished()) { return true; }
	return false;
}

bool NormalAttackState::IsCancelable() const
{
	if (currentSkill_ && currentSkill_->IsCancelable()) { return true; }
	return false;
}


/*==================================================*/
/* 特殊攻撃状態 */
/*==================================================*/
void SpecialAbilityState::Enter()
{
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->ExecuteSpecialAbility();

	// playerの通常攻撃タイプを取得
	currentSkill_ = player_->GetAbilitySkill();

	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Enter(player_);
	}
}

void SpecialAbilityState::Update()
{
	if (currentSkill_) { currentSkill_->Update(player_); }
}

void SpecialAbilityState::Exit()
{
	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Exit(player_);
	}
	// 借りていたポインタを空にする（deleteはされないので安全）
	currentSkill_ = nullptr;
}


bool SpecialAbilityState::IsFinished() const
{
	if (currentSkill_ && currentSkill_->IsFinished()) { return true; }
	return false;
}

bool SpecialAbilityState::IsCancelable() const
{
	if (currentSkill_ && currentSkill_->IsCancelable()) { return true; }
	return false;
}



/*==================================================*/
/* 特殊行動状態 */
/*==================================================*/
void UtilityState::Enter()
{
	// playerの通常攻撃タイプを取得
	currentSkill_ = player_->GetUtilitySkill();

	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Enter(player_);
	}
}

void UtilityState::Update()
{
	if (currentSkill_) { currentSkill_->Update(player_); }
}

void UtilityState::Exit()
{
	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Exit(player_);
	}
	// 借りていたポインタを空にする（deleteはされないので安全）
	currentSkill_ = nullptr;

}

bool UtilityState::IsFinished() const
{
	if (currentSkill_ && currentSkill_->IsFinished()) { return true; }
	return false;
}

bool UtilityState::IsCancelable() const
{
	if (currentSkill_ && currentSkill_->IsCancelable()) { return true; }
	return false;
}
/*==================================================*/
/* 死亡状態 */
/*==================================================*/

void DeadState::Enter()
{
	// 死亡アニメーション
	player_->PlayAnimation(static_cast<int>(StateID::Dead));
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
