#include "stdafx.h"
#include "PlayerState.h"
#include "Player.h"
#include "ActorStatus.h"
#include "src/Battle/MissionManager.h"
#include "src/Skill/NormalAttack/NormalAtatck.h"
#include "src/Skill/SpecialAbility/AbilityBase.h"
#include "src/Skill/Utility/Utility.h"
#include "src/collision/GhostBody.h"

#include "src/Sound/SoundManager.h"




/*==================================================*/
/******************** 待機状態 **********************/
/*==================================================*/

void IdleState::Enter()
{
	// 待機アニメーション
	player_->PlayAnimation(static_cast<int>(PlayerStateID::Idle));

	// スタミナステートの変更
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enRecover);
}

void IdleState::Update()
{
	// 移動速度はゼロに
	player_->SetMoveVelocity(Vector3::Zero);
}

void IdleState::Exit()
{
	// スタミナステートを回復しないように変更
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enNone);
}


/*==================================================*/
/******************** 歩き状態 **********************/
/*==================================================*/

void WalkState::Enter()
{
	// 歩きアニメーション
	player_->PlayAnimation(static_cast<int>(PlayerStateID::Walk));

	// スタミナステートの変更
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enRecover);

	// ベーススピードを取得
	baseMoveSpeed_ = status->GetMoveSpeedBase();

	// 枯渇状態なら速度を下げる
	if (status->IsExhausted()) {
		baseMoveSpeed_ *= status->GetExhaustedSpeedRate();
	}

	// SE
	{
		const auto* p = ParameterManager::Get().GetPlayerStateParam();
		taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
		const int id = taskScheduler_->CreateLoopSequence(p->walkSeInterval);
		taskScheduler_->AddLoopTimer(id, p->walkLoopTime, [&](void) {
			SoundManager::Get().PlaySE(enSoundKind_Player_Walk);
			});
	}
}

void WalkState::Update()
{
	// 共通関数に歩く時の基本スピードを渡す
	Vector3 velocity = CalcMovementVelocity(baseMoveSpeed_);

	// 移動先をPlayerに渡す
	player_->SetMoveVelocity(velocity);

	// 歩きのSEを鳴らす
	{
		taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	}
}

void WalkState::Exit()
{
	taskScheduler_.reset();
	baseMoveSpeed_ = 0.0f;

	// スタミナステートを回復しないように変更
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enNone);
}


/*==================================================*/
/* 走る状態 */
/*==================================================*/

void RunState::Enter()
{
	// 走るアニメーション
	player_->PlayAnimation(static_cast<int>(PlayerStateID::Run));

	// スタミナステートの変更
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enDrain);

	const auto* p = ParameterManager::Get().GetPlayerStateParam();
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();
	// SEの再生
	{
		const int id = taskScheduler_->CreateLoopSequence(p->runSeInterval);
		taskScheduler_->AddLoopTimer(id, p->walkLoopTime, [&](void) {
			SoundManager::Get().PlaySE(enSoundKind_Player_Walk);
			});
	}

	// エフェクト
	{
		const Vector3 effectScale = Vector3(p->walkEffectScale);
		const int id = taskScheduler_->CreateLoopSequence(p->runSeInterval);
		taskScheduler_->AddLoopTimer(id, p->walkLoopTime, [&, effectScale](void) {
			const Vector3 playerPos = player_->GetTransformPosition();
			const Quaternion playerRot = player_->GetStateMachine()->GetRotation();
			EffectManager::Get().PlayEffect(enEffectKind_Dash_Wind, playerPos, playerRot, effectScale);
			});
	}
}

void RunState::Update()
{
	// プレイヤーの走るときのベース速度を取得
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	float moveSpeed = status->GetRunSpeedBase();

	// 共通関数に歩く時の基本スピードを渡す
	Vector3 velocity = CalcMovementVelocity(moveSpeed);

	// 移動先をPlayerに渡す
	player_->SetMoveVelocity(velocity);

	// タスクスケジュールの時間を進める
	{
		taskScheduler_->Update(g_gameTime->GetFrameDeltaTime());
	}
}

void RunState::Exit()
{
	taskScheduler_.reset();

	// スタミナステートを回復しないように変更
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enNone);
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

	// ミッションに使用したことを報告
	const PlayerParam& playerParam = CharacterDataBase::Get().GetPlayerParam();
	MissionManager::Get().NotifyNormalAttackUsed(playerParam.nAttack);
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

	// ミッションに使用したことを報告
	const PlayerParam& playerParam = CharacterDataBase::Get().GetPlayerParam();
	MissionManager::Get().NotifyAbilityUsed(playerParam.ability);
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
/* 汎用行動状態 */
/*==================================================*/

void UtilityState::Enter()
{
	// スタミナを消費する
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	const float staminaCost = status->GetSkillStatus().GetDecreaseStamina("Utility");
	status->ConsumeStamina(staminaCost);

	// playerの通常攻撃タイプを取得
	currentSkill_ = player_->GetUtilitySkill();

	// スキルに処理を丸投げ！
	if (currentSkill_) {
		currentSkill_->Enter(player_);
	}

	// ミッションに使用したことを報告
	const PlayerParam& playerParam = CharacterDataBase::Get().GetPlayerParam();
	MissionManager::Get().NotifyUtilityUsed(playerParam.utility);
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

	// スタミナ消費を止める
	auto* status = player_->GetStatus()->As<PlayerStatus>();
	status->SetStaminaState(PlayerStatus::StaminaState::enNone);
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

void DeathState::Enter()
{
	// 死亡アニメーション
	player_->PlayAnimation(static_cast<int>(PlayerStateID::Dead));

	isFinished_ = false; //!< 現在のステート処理が終わったか

	// タスクスケジューラを作成
	taskScheduler_ = std::make_unique<TaskSchedulerSystem>();

	// アニメーションが再生されてから特定の時間たったら
	const float deathTime = ParameterManager::Get().GetPlayerStateParam()->deathAnimationTime;
	taskScheduler_->AddTimer(deathTime, [&]()
		{
			PlayerStatus* status = player_->GetStatus()->As<PlayerStatus>();
			status->Die();
		});
}

void DeathState::Update()
{
	// 移動速度を0に
	player_->SetMoveVelocity(Vector3::Zero);

	// 時間を計算
	if (taskScheduler_) { taskScheduler_->Update(g_gameTime->GetFrameDeltaTime()); }
}

void DeathState::Exit()
{
	taskScheduler_.reset();
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
	if (stickLAmount < ParameterManager::Get().GetPlayerStateParam()->moveThreshold) { return Vector3::Zero; }

	// 方向を取得
	Vector3 dir = stateMachine->GetDirection();

	// スピードとフレームレート(DeltaTime)を掛けて返す
	return dir * speed * g_gameTime->GetFrameDeltaTime();
}
