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
	AddState(BossStateID::Jump, new HitStampState(this));
	AddState(BossStateID::Spin, new SpinState(this));
	AddState(BossStateID::Clicked_Gollira, new ThrowRockState(this));
	AddState(BossStateID::Clicked_Turtle, new LaserState(this));
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
	BossStatus* status = new BossStatus();
	status_ = status;
}

BossCharacter::~BossCharacter()
{
	// statusを破棄
	delete status_;
	status_ = nullptr;
}

bool BossCharacter::Start()
{
	// ステータスの初期化
	status_->As<BossStatus>()->Init(param_.characterKey_);
	auto initParam = status_->As<BossStatus>()->GetInitParam();

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
		param_.modelAxis_
	);

	// TODO : プレイヤーとの距離を話すため
	transform_.localPosition = initParam.pos_;
	transform_.localRotation = initParam.rot_;
	transform_.localScale = initParam.scal_;
	transform_.UpdateTransform();

	modelRender_.SetPosition(transform_.position);
	modelRender_.SetRotation(transform_.rotation);
	modelRender_.SetScale(transform_.scale);

	// 現在のステートをIdleに設定
	ChangeState(BossStateID::Idle);

	// キャラコン
	{
		charaCon_.Init(initParam.charaConRadius_, initParam.charaConHeight_, transform_.position);
	}

	// コリジョン作成
	{
		damageBody_ = std::make_unique<GhostBody>();
		damageBody_->CreateCapsule(this, CharacterID::BossID(), initParam.collisionRadius_,initParam.collisionHeight_, ghost::CollisionAttribute::BossDef, ghost::CollisionAttributeMask::Boss);
		damageBody_->SetPosition(transform_.position);
	}
	return true;
}

void BossCharacter::Update()
{
	// アクティブでないなら更新しない
	if (!isUpdate_) return; 


	// ボスの移動処理
	if(!isMoveStop_)
	{
		// 移動処理
		transform_.localPosition += moveVelocity_;

		// 回転処理
		transform_.localRotation = targetPlayerRot_;

		// トランスフォームの更新
		transform_.UpdateTransform();

		// キャラコン
		{
			Vector3 xzTarget = transform_.position;
			xzTarget.y = charaCon_.GetPosition().y; // Yはキャラコン内部値を維持（変化させない）
			Vector3 conResult = charaCon_.Execute(xzTarget, g_gameTime->GetFrameDeltaTime());

			// XZだけ反映、YはState側が管理するので触らない
			transform_.localPosition.x = conResult.x;
			transform_.localPosition.z = conResult.z;
		}

		// 重力
		{
			if (transform_.localPosition.y < 0.0f)
			{
				Vector3 pos = transform_.localPosition;
				pos.y = 0.0f;
				transform_.localPosition = pos;
				charaCon_.SetPosition(pos); // キャラコン内部の座標も合わせる
			}
		}

		transform_.UpdateTransform();

		// モデルへ反映
		modelRender_.SetPosition(transform_.position);
		modelRender_.SetRotation(transform_.rotation);
		modelRender_.Update();
	}
	
	// ゴーストコリジョン更新
	{
		Vector3 collisionHeightUp = status_->As<BossStatus>()->GetInitParam().collisionHeightUpValue_;
		Vector3 collisionPos = transform_.position + collisionHeightUp;
		damageBody_->SetPosition(collisionPos);
	}
	// 更新
	{
		// ボスのステータス更新
		BossStatus* status = status_->As<BossStatus>();
		if (status) { status->Update(); }

		modelRender_.Update();// モデルの更新
		if (currentState_) { currentState_->Update(); } // 現在のステートがある場合、現在のステートの更新
	}
}

void BossCharacter::Render(RenderContext& rc)
{
	modelRender_.Draw(rc);
}
