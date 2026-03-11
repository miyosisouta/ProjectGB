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
		AnimationData{"Assets/animData/idle.tka",true},
		AnimationData{"Assets/animData/walk.tka",true},
		AnimationData{"Assets/animData/run.tka",true},
		AnimationData{"Assets/animData/jump.tka",true},
		// アニメーションを増やすときはここから
	};

	constexpr uint8_t ANIMATION_IDLE = 0; // 待機
	constexpr uint8_t ANIMATION_WALK = 1; // 歩く
	constexpr uint8_t ANIMATION_RUN = 2;  // 走る
}


void Player::SetUpTranslateRulu()
{
	// ステート（状態）を登録
	stateMachine_.AddState(StateID::Idle, new IdleState(this));
	stateMachine_.AddState(StateID::Walk, new WalkState(this));
	stateMachine_.AddState(StateID::Run, new RunState(this));
	stateMachine_.AddState(StateID::Dead, new DeadState(this));

	// トランジション（遷移ルール）を登録
	stateMachine_.AddGlobalTransition([]() { if (g_pad[0]->IsPress(enButtonX)) { return true; } return false; }, StateID::Dead); /* HPが0なら他のステート関係なく実行 */
	stateMachine_.AddTransition(StateID::Idle, StateID::Walk, []() // 待機中の場合に左スティック入力があれば歩きステートへ変更
		{
			if(g_pad[0]->IsPress(enButtonA)){ return true; }
			return false;
		});
	stateMachine_.AddTransition(StateID::Idle, StateID::Run, []()
		{
			if (g_pad[0]->IsPress(enButtonY)) { return true; }
			return false;
		});
	stateMachine_.AddTransition(StateID::Walk, StateID::Idle, []()
		{
			if (g_pad[0]->IsPress(enButtonB)) { return true; }
			return false;
		});
	stateMachine_.AddTransition(StateID::Walk, StateID::Run, []()
		{
			if (g_pad[0]->IsPress(enButtonY)) { return true; }
			return false;
		});
	stateMachine_.AddTransition(StateID::Run, StateID::Idle, []()
		{
			if (g_pad[0]->IsPress(enButtonB)) { return true; }
			return false;
		});
	stateMachine_.AddTransition(StateID::Run, StateID::Walk, []()
		{
			if (g_pad[0]->IsPress(enButtonA)) { return true; }
			return false;
		});
}

void Player::PlayAnimation(const StateID id)
{
	// StateID と アニメーションのインデックス(sAnimPathsの並び順)を紐づける
	int animIndex = 0;
	switch (id) {
	case StateID::Idle: animIndex = anim::ANIMATION_IDLE; break;	// 待機。
	case StateID::Walk: animIndex = anim::ANIMATION_WALK; break;	// 歩き。
	case StateID::Run:  animIndex = anim::ANIMATION_RUN; break;		// 走る。
	
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
			"Assets/modelData/unityChan.tkm",	// ファイルパス
			animationClipList_.data(),			// アニメーションデータ
			animationClipList_.size(),			// リストの数
			enModelUpAxisY						// モデルの上方向
		);

		// モデルの座標を更新・初期化
		{
			modelRender_.SetPosition(transform_.m_position);
			modelRender_.SetScale(transform_.m_scale);
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
}


void Player::Render(RenderContext& rc)
{
	// 共通処理を呼び出す
	Character::Render(rc);
}