#include "stdafx.h"
#include "BossCharacter.h"
#include "NPCController.h"
#include "BossState.h"
#include "src/Actor/ActorStatus.h"


/** ===================================================== */
/** アニメーション関連 */
/** ===================================================== */

void BossCharacter::SetupAnimation()
{
	// アニメーションの読み込み
	const int animCount = BossAnimID::enAnimNum; // アニメーションの総数
	animationClipList_.Create(animCount); // メモリをあらかじめ確保

	for (int i = 0; i < animCount; ++i)
	{
		// filePath が空っぽじゃない（ちゃんとセットされている）時だけ読み込む
		if (!param_.anims[i].filePath.empty())
		{
			// c_str()を使い、stringからconst charに変える
			animationClipList_[i].Load(param_.anims[i].filePath.c_str());
			animationClipList_[i].SetLoopFlag(param_.anims[i].isLoop);
		}
	}
}

void BossCharacter::PlayAnimation(const int id,const int animSpeed)
{
	modelRender_.PlayAnimation(id); // Idをもとにそのアニメーションの再生
	modelRender_.SetAnimationSpeed(animSpeed);
}

/** ===================================================== */
/** ステート関連 */
/** ===================================================== */

void BossCharacter::SetupTranslate()
{
	AddState(BossStateID::Idle, new BossIdleState(this));
	AddState(BossStateID::Run, new BossRunState(this));
	AddState(BossStateID::Attack, new BossAttackState(this));
	AddState(BossStateID::Death, new BossDeathState(this));
}

void BossCharacter::ChangeState(BossStateID nextStateId)
{
	// 今と同じ状態なら何もしない（ストッパー）
	if (currentState_ && currentStateID_ == nextStateId) { return; }

	// 現在のフェーズ終了処理
	if (currentState_) { currentState_->Exit();	}

	// 次のステートを見つけて切り替え
	auto* nextState = FindState(nextStateId);
	if (nextState)
	{
		nextState->Enter();
		currentState_ = nextState;
		currentStateID_ = nextStateId;
	}
}


/** ===================================================== */
/** BossCharacterのコンストラクタなど */
/** ===================================================== */

BossCharacter::BossCharacter()
{
	auto* bossStatus = new BossStatus();
	status_ = bossStatus;
}

BossCharacter::~BossCharacter()
{
	// statusを破棄
	delete status_;
	status_ = nullptr;
}

bool BossCharacter::Start()
{
	// セットアップ
	{
		SetupTranslate(); // ステートクラスのインスタンス作成
		SetupAnimation(); // アニメーションのロード
	}
	
	// モデルの初期化
	modelRender_.Init(
		param_.modelPath_.c_str(),
		animationClipList_.data(),
		animationClipList_.size(),
		enModelUpAxisZ
	);

	// TODO : プレイヤーとの距離を話すため
	transform_.localPosition = Vector3(-300.0f, 0.0f, 0.0f);

	// モデルの設定
	transform_.localScale = Vector3(2.5f, 2.5f, 2.5f);
	transform_.UpdateTransform();
	modelRender_.SetPosition(transform_.position);
	modelRender_.SetRotation(transform_.rotation);
	modelRender_.SetScale(transform_.scale);

	// 現在のステートをIdleに設定
	ChangeState(BossStateID::Idle);

	// コリジョン作成
	{
		damageBody_ = std::make_unique<GhostBody>();
		damageBody_->CreateCapsule(this, CharacterID::BossID(), 110.0f, 150.0f, ghost::CollisionAttribute::BossDef, ghost::CollisionAttributeMask::Boss);
		damageBody_->SetPosition(transform_.position);
	}

	// ボスのステータスを作って、確定した数値を流し込む！
	BossParam param;
	BossStatus* status = new BossStatus();
	status->InitStatus(param.maxHp_, param.attack_);

	return true;
}

void BossCharacter::Update()
{
	// ボスの移動処理
	if(!isMoveStop_)
	{
		// 移動処理
		transform_.localPosition += moveVelocity_;

		// 回転処理
		transform_.localRotation = targetPlayerRot_;

		// トランスフォームの更新
		transform_.UpdateTransform();

		// モデルへ反映
		modelRender_.SetPosition(transform_.position);
		modelRender_.SetRotation(transform_.rotation);
		modelRender_.Update();
	}

	// コリジョン更新
	{
		Vector3 collisionPos = transform_.position + Vector3(0.0f, 120.0f, 0.0f);
		damageBody_->SetPosition(collisionPos);

	}
	// 更新
	{
		// ボスのステータス更新
		BossStatus* status = status_->As<BossStatus>();
		status->Update();

		modelRender_.Update();// モデルの更新
		if (currentState_) { currentState_->Update(); } // 現在のステートがある場合、現在のステートの更新
	}
}

void BossCharacter::Render(RenderContext& rc)
{
	modelRender_.Draw(rc);
}
