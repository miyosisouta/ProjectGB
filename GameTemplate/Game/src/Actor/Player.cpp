#include "stdafx.h"
#include "Player.h"
#include "StateMachine.h"
#include "PlayerState.h"


/*
 * アニメーションの名前空間
 */
namespace anim
{
	// アニメーション設定
	struct AnimationData
	{
		const char* filePath; // ファイルパス
		bool loopFlag; // ループするかフラグ
	};

	// 読み込みたいアニメーションのファイルパスを並べる
	// ここにパスを追加・削除するだけで、スタート処理にてm_animationClipListに自動で追加される
	static AnimationData sAnimPaths[] = {
		AnimationData{"Assets/Objects/Player/Animation/IdleA.tka",false},
		AnimationData{"Assets/Objects/Player/Animation/Walk.tka",true},
		AnimationData{"Assets/Objects/Player/Animation/Run.tka",true},
		AnimationData{"Assets/Objects/Player/Animation/Attack.tka",false},
		AnimationData{"Assets/Objects/Player/Animation/Death.tka",false}
		// アニメーションを増やすときはここから
	};

	constexpr uint8_t ANIMATION_IDLE = 0; // 待機
	constexpr uint8_t ANIMATION_WALK = 1; // 歩く
	constexpr uint8_t ANIMATION_RUN = 2;  // 走る
	constexpr uint8_t ANIMATION_NORMAL_ATTACK = 3;  // 通常攻撃
}


void Player::SetUpTranslateRulu()
{
	// ステート（状態）を登録
	{
		stateMachine_.AddState(StateID::Idle, new IdleState(this));
		stateMachine_.AddState(StateID::Walk, new WalkState(this));
		stateMachine_.AddState(StateID::Run, new RunState(this));
		stateMachine_.AddState(StateID::NormalAttack, new NormalAttackState(this));
		stateMachine_.AddState(StateID::Dead, new DeadState(this));
	}


	// トランジション（遷移ルール）を登録
	{
		// 優先される条件
		{
			stateMachine_.AddGlobalTransition([this]() { if (IsDead()) { return true; } return false; }, StateID::Dead); /* HPが0なら他のステート関係なく実行 */
		}

		// 一般ルール 
		{
			/** 待機 */
			{
				// 待機 -> 歩き
				stateMachine_.AddTransition(StateID::Idle, StateID::Walk, [this]() {
					return stateMachine_.GetStickLAmount() > 0.01f && !stateMachine_.IsDash();
					});

				// 待機 -> 走り
				stateMachine_.AddTransition(StateID::Idle, StateID::Run, [this]() {
					return stateMachine_.GetStickLAmount() > 0.01f && stateMachine_.IsDash();
					});

				// 待機 -> 通常攻撃
				stateMachine_.AddTransition(StateID::Idle, StateID::NormalAttack, [this]() {
					return stateMachine_.IsActionButtonB();
					});
			}

			/** 歩き */
			{
				// 歩き -> 待機
				stateMachine_.AddTransition(StateID::Walk, StateID::Idle, [this]() {
					return stateMachine_.GetStickLAmount() < 0.01f;
					});

				// 歩き -> 走り
				stateMachine_.AddTransition(StateID::Walk, StateID::Run, [this]() {
					return stateMachine_.IsDash();
					});

				// 歩き -> 通常攻撃
				stateMachine_.AddTransition(StateID::Walk, StateID::NormalAttack, [this]() {
					return stateMachine_.IsActionButtonB();
					});
			}

			/** 走り */
			{
				// 走り -> 待機
				stateMachine_.AddTransition(StateID::Run, StateID::Idle, [this]() {
					return stateMachine_.GetStickLAmount() < 0.01f;
					});

				// 走り -> 歩き
				stateMachine_.AddTransition(StateID::Run, StateID::Walk, [this]() {
					return !stateMachine_.IsDash();
					});

				// 走り -> 通常攻撃
				stateMachine_.AddTransition(StateID::Run, StateID::NormalAttack, [this]() {
					return stateMachine_.IsActionButtonB();
					});
			}

			/** 通常攻撃 */
			{
				// 通常攻撃 → 待機
				stateMachine_.AddTransition(StateID::NormalAttack, StateID::Idle, [this]() {
					auto* currentState = static_cast<PlayerStateBase*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsFinished();
					});

				// 通常攻撃 → 歩き
				stateMachine_.AddTransition(StateID::NormalAttack, StateID::Walk, [this]() {
					auto* currentState = static_cast<PlayerStateBase*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsFinished();
					});

				// 通常攻撃 → 走る
				stateMachine_.AddTransition(StateID::NormalAttack, StateID::Run, [this]() {
					auto* currentState = static_cast<PlayerStateBase*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsCancelable() && stateMachine_.IsDash();
					});
			}
		}
	}
}


void Player::PlayAnimation(const StateID id)
{
	// StateID と アニメーションのインデックス(sAnimPathsの並び順)を紐づける
	int animIndex = 0;
	switch (id) {
	case StateID::Idle: animIndex = anim::ANIMATION_IDLE; break;	// 待機。
	case StateID::Walk: animIndex = anim::ANIMATION_WALK; break;	// 歩き。
	case StateID::Run:  animIndex = anim::ANIMATION_RUN; break;		// 走る。
	case StateID::NormalAttack:  animIndex = anim::ANIMATION_NORMAL_ATTACK; break;		// 走る。
	
	default: return; // ないなら処理を返す
	}

	modelRender_.PlayAnimation(animIndex); // Idをもとにそのアニメーションの再生
}


Player::Player()
{
}

bool Player::Start()
{
	// 自分はPlayerグループであることを宣言する
	SetUpdateGroup(UpdateGroup::Player);

	// アニメーション
	{
		// アニメーションクリップリストの配列数を決定
		const size_t animCount = ARRAYSIZE(anim::sAnimPaths);// アニメーションの数を数える
		animationClipList_.Create(animCount); // アニメーションクリップの配列を作る

		// すべてのアニメーションを登録
		for (size_t i = 0; i < animCount; ++i)
		{
			animationClipList_[i].Load(anim::sAnimPaths[i].filePath); // ファイルパスを読み込む
			animationClipList_[i].SetLoopFlag(anim::sAnimPaths[i].loopFlag); // ループ設定
		}
	}
	

	// モデルの設定
	{
		// モデルのファイルパスとアニメーションを設定
		modelRender_.Init(
			"Assets/Objects/Player/Model/Model.tkm",	// ファイルパス
			animationClipList_.data(),			// アニメーションデータ
			animationClipList_.size(),			// リストの数
			enModelUpAxisZ						// モデルの上方向
		);

		// モデルの座標を更新・初期化
		{
			transform_.UpdateTransform();
			modelRender_.SetPosition(transform_.position);
			modelRender_.SetRotation(transform_.rotation);
			modelRender_.SetScale(transform_.scale);
			modelRender_.Update();
		}
	}

	// ステート遷移のルール設定
	SetUpTranslateRulu();


	// 共通処理を呼び出す
	return Character::Start();
}


void Player::Update()
{
	// 共通処理を呼び出す : ステートマシンのアップデートを呼んでます
	Character::Update();

	// モデルへの設定
	{
		// 移動処理
		transform_.localPosition += moveVelocity_;

		// 回転処理
		transform_.localRotation = stateMachine_.GetRotation();

		// トランスフォームの更新
		transform_.UpdateTransform();

		// モデルへ反映
		modelRender_.SetPosition(transform_.position);
		modelRender_.SetRotation(transform_.rotation);
		modelRender_.Update();
	}
}


void Player::Render(RenderContext& rc)
{
	// 共通処理を呼び出す
	Character::Render(rc);
}