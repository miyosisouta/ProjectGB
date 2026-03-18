#include "stdafx.h"
#include "Player.h"
#include "StateMachine.h"
#include "PlayerState.h"
#include "src/Skill/ISkill.h"
#include "src/Skill/NormalAttack/NormalAtatck.h"  
#include "src/Skill/SpecialAbility/DefaultAttack.h"
#include "src/Skill/SpecialAbility/Guard.h"
#include "src/Skill/SpecialAbility/Magic.h"
#include "src/Skill/SpecialAbility/Bomb.h"
#include "src/Skill/Utility/Utility.h"



/* ==================================== */
/* アニメーション */
/* ==================================== */

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
	constexpr uint8_t ANIMATION_SPECIAL_ABILITY = 3; // 特殊能力
}

void Player::PlayAnimation(const int id)
{
	// StateID と アニメーションのインデックス(sAnimPathsの並び順)を紐づける
	int animIndex = 0;
	switch (static_cast<StateID>(id)) {
	case StateID::Idle: animIndex = anim::ANIMATION_IDLE; break;	// 待機。
	case StateID::Walk: animIndex = anim::ANIMATION_WALK; break;	// 歩き。
	case StateID::Run:  animIndex = anim::ANIMATION_RUN; break;		// 走る。
	case StateID::NormalAttack:  animIndex = anim::ANIMATION_NORMAL_ATTACK; break;		// 通常攻撃。
	case StateID::SpecialAbility:  animIndex = anim::ANIMATION_SPECIAL_ABILITY; break;	// 特殊能力。

	default: return; // ないなら処理を返す
	}

	modelRender_.PlayAnimation(animIndex); // Idをもとにそのアニメーションの再生
}



/* ==================================== */
/* セットアップ */
/* ==================================== */

bool Player::CanSpecialAbility()
{
	if (stateMachine_.IsActionButtonY()) {
		if (specialAblityCoolTime_ <= 0.0f) {
			return true;
		}
	}
	
	return false;
}

void Player::SetUpTranslateRulu()
{
	// ステート（状態）を登録
	{
		stateMachine_.AddState(StateID::Idle, new IdleState(this));
		stateMachine_.AddState(StateID::Walk, new WalkState(this));
		stateMachine_.AddState(StateID::Run, new RunState(this));
		stateMachine_.AddState(StateID::NormalAttack, new NormalAttackState(this));
		stateMachine_.AddState(StateID::SpecialAbility, new SpecialAbilityState(this));
		stateMachine_.AddState(StateID::Utility, new UtilityState(this));
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
				// 待機 -> 特殊能力
				stateMachine_.AddTransition(StateID::Idle, StateID::SpecialAbility, [this]() {
					// TODO :: 後で変えます
					if (CanSpecialAbility()) 
					{ 
						specialAblityCoolTime_ = specialAblityCoolDown_;
						return true;
					}
					return false;
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
				// 歩き -> 特殊能力
				stateMachine_.AddTransition(StateID::Walk, StateID::SpecialAbility, [this]() {
					// TODO :: 後で変えます
					if (CanSpecialAbility())
					{
						specialAblityCoolTime_ = specialAblityCoolDown_;
						return true;
					}
					return false;
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

				// 走り -> 特殊能力
				stateMachine_.AddTransition(StateID::Run, StateID::SpecialAbility, [this]() {
					return CanSpecialAbility();
					});
			}

			/** 通常攻撃 */
			{
				// 通常攻撃 → 待機
				stateMachine_.AddTransition(StateID::NormalAttack, StateID::Idle, [this]() {
					auto* currentState = static_cast<NormalAttackState*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsFinished();
					});
			}

			/* 特殊能力 */
			{
				// 特殊能力 → 待機
				stateMachine_.AddTransition(StateID::SpecialAbility, StateID::Idle, [this]() {
					auto* currentState = static_cast<NormalAttackState*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsFinished();
					});
			}
		}
	}
}

void Player::CreateSkill(NormalAttackType nAttackType, AbilityType abilityType, UtilityType utilityType)
{
	EquipNormalAttack(nAttackType);
	EquipAbility(abilityType);
	EquipUtility(utilityType);
}


/* ==================================== */
/* 更新・描画処理 */
/* ==================================== */
Player::Player()
{
}
Player::~Player()
{
}
bool Player::Start()
{
	// 自分はPlayerグループであることを宣言する
	// SetUpdateGroup(UpdateGroup::Player);

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

	if (specialAblityCoolTime_ > 0.0f) { specialAblityCoolTime_ -= g_gameTime->GetFrameDeltaTime();	}
	else { specialAblityCoolTime_ = 0.0f; }
}

void Player::Render(RenderContext& rc)
{
	// 共通処理を呼び出す
	Character::Render(rc);
}



/* ==================================== */
/* スキルの設定・生成 */
/* ==================================== */

void Player::EquipNormalAttack(NormalAttackType type)
{
	

	switch (type)
	{
	case NormalAttackType::enBite:
		activeNormalAttack_ = std::make_unique<Bite>();
		break;
	default:
		activeNormalAttack_.reset(); // 何も装備していない状態
		break;
	}
}

void Player::EquipAbility(AbilityType type) 
{

	switch (type) {
		// --- ガード関連 ---
	case AbilityType::enGuard:
		// activeAbility_ = std::make_unique<SkillGuard>(); 
		break;
	case AbilityType::enReflectiveGuard:
		// activeAbility_ = std::make_unique<SkillReflectiveGuard>();
		break;
	case AbilityType::enCounter:
		// activeAbility_ = std::make_unique<SkillCounter>();
		break;

		// --- 魔法関連 ---
	case AbilityType::enFireMagic:
		// activeAbility_ = std::make_unique<SkillFireMagic>();
		break;
	case AbilityType::enFireMagic_Strong:
		// activeAbility_ = std::make_unique<SkillFireMagicStrong>();
		break;
	case AbilityType::enAbsorption:
		// activeAbility_ = std::make_unique<SkillAbsorption>();
		break;

		// --- 爆弾関連 ---
	case AbilityType::enBomb:
		// activeAbility_ = std::make_unique<SkillBomb>();
		break;
	case AbilityType::enBomb_Decoy:
		// activeAbility_ = std::make_unique<SkillBombDecoy>();
		break;
	case AbilityType::enLandmine:
		// activeAbility_ = std::make_unique<SkillLandmine>();
		break;
	default:
		activeAbility_ = std::make_unique<DefaultAttack>();
		break;
	}
}

void Player::EquipUtility(UtilityType type) 
{
	switch (type) {
	case UtilityType::enDodgeRoll:
		// activeUtility_ = std::make_unique<SkillDodgeRoll>();
		break;
	default:
		activeUtility_.reset();
		break;
	}
}