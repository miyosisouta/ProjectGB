#include "stdafx.h"
#include "TutorialBossScript.h"
#include "src/Actor/BossCharacter.h"
#include "src/Core/ParameterManager.h"


void TutorialBossScript::Init(BossCharacter* boss)
{
	boss_ = boss;

	// 登場演出用にボスの初期位置・向きを覚えておく（ResetToSpawn()で使う）
	if (boss_)
	{
		spawnPosition_ = boss_->GetTransformPosition();
		spawnRotation_ = boss_->GetTransformRotation();
	}
}


void TutorialBossScript::Update(float deltaTime, bool shouldBeActive, const Vector3& targetPos, bool isFreeStage)
{
	if (!boss_) { return; }

	if (!shouldBeActive)
	{
		scriptActive_ = false;
		return;
	}

	// AI(NPCController)を無効化したままなので、狙う位置は自前で毎フレーム更新する
	boss_->SetTargetPos(targetPos);

	if (!scriptActive_)
	{
		// 開始した瞬間：SetControlEnabled(false)で止めていた移動・回転を解除し、最初のHitStamp攻撃を出す
		scriptActive_ = true;
		waitingIdle_  = false;
		boss_->SetMoveStop(false);
		boss_->ChangeState(BossStateID::HitStamp);
		return;
	}

	if (waitingIdle_)
	{
		idleElapsed_ += deltaTime;

		// マジックナンバー・時間設定はTutorialParameter.jsonから読み込む
		const auto* param = ParameterManager::Get().GetTutorialParam();
		const float idleDuration = isFreeStage ? param->bossIdleDurationInFree : param->bossIdleDurationInTeaching;
		if (idleElapsed_ >= idleDuration)
		{
			waitingIdle_ = false;
			boss_->ChangeState(BossStateID::HitStamp);
		}
		return;
	}

	// 攻撃中：アニメーションが終わったらIdleへ戻し、待機を開始する
	if (boss_->GetCurrentStateID() == BossStateID::HitStamp && boss_->IsCurrentStateFinished())
	{
		boss_->ChangeState(BossStateID::Idle);
		waitingIdle_ = true;
		idleElapsed_ = 0.0f;
	}
}


void TutorialBossScript::ResetToSpawn()
{
	if (!boss_) { return; }

	scriptActive_ = false;
	waitingIdle_  = false;
	boss_->SetMoveStop(true);
	boss_->ChangeState(BossStateID::Idle);
	boss_->SetTransformImmediate(spawnPosition_, spawnRotation_);
}
