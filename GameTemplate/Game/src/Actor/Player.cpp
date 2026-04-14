#include "stdafx.h"
#include "Player.h"
#include "Character.h"
#include "ActorStatus.h"
#include "StateMachine.h"
#include "PlayerState.h"
#include "src/Skill/ISkill.h"
#include "src/Skill/NormalAttack/NormalAtatck.h"  
#include "src/Skill/SpecialAbility/DefaultAttack.h"
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
		AnimationData{"Assets/Objects/Player/Animation/IdleA.tka",false},	// 待機
		AnimationData{"Assets/Objects/Player/Animation/Walk.tka",true},		// 歩き
		AnimationData{"Assets/Objects/Player/Animation/Run.tka",true},		// 走る
		AnimationData{"Assets/Objects/Player/Animation/Attack.tka",false},	// 攻撃
		AnimationData{"Assets/Objects/Player/Animation/Roll.tka",false},	// 回転
		AnimationData{"Assets/Objects/Player/Animation/Fear.tka",false},	// 地雷
		AnimationData{"Assets/Objects/Player/Animation/Eat.tka",false},		// 魔法
		AnimationData{"Assets/Objects/Player/Animation/Death.tka",false}	// 死亡
		// アニメーションを増やすときはここから
	};

	constexpr uint8_t ANIMATION_IDLE = 0; // 待機
	constexpr uint8_t ANIMATION_WALK = 1; // 歩く
	constexpr uint8_t ANIMATION_RUN = 2;  // 走る
	constexpr uint8_t ANIMATION_NORMAL_ATTACK = 3;  // 通常攻撃
	constexpr uint8_t ANIMATION_SPECIAL_ABILITY = 3; // 特殊能力
	constexpr uint8_t ANIMATION_AVOID = 4; // 汎用スキル
}

namespace 
{
	/* 通常攻撃の変換 */
	const char* ToNormalAttackKey(NormalAttackType type)
	{
		switch (type)
		{
		case NormalAttackType::enBite: return "Bite";
		default:                       return nullptr;
		}
	}
	/* 特殊攻撃の変換 */
	const char* ToAbilityKey(AbilityType type)
	{
		switch (type)
		{
		case AbilityType::enDefault:         return "DefaultAttack";
		case AbilityType::enLandmine:        return "Landmine";
		case AbilityType::enFireMagic:       return "Magic";
		default:                             return nullptr;
		}
	}
	/* 汎用スキルの変換 */
	const char* ToUtilityKey(UtilityType type)
	{
		switch (type)
		{
		case UtilityType::enAvoid: return "Dodge";
		default:                   return nullptr;
		}
	}

	/* コリジョン */
	constexpr float COLLISION_RADIUS = 30.0f;	// 半径
	constexpr float COLLISION_HEIGHT = 100.0f;	// 高さ
}

void Player::PlayAnimation(const int id)
{
	// StateID と アニメーションのインデックス(sAnimPathsの並び順)を紐づける
	int animIndex = 0;
	switch (static_cast<PlayerStateID>(id)) {
	case PlayerStateID::Idle: animIndex = anim::ANIMATION_IDLE; break;	// 待機。
	case PlayerStateID::Walk: animIndex = anim::ANIMATION_WALK; break;	// 歩き。
	case PlayerStateID::Run:  animIndex = anim::ANIMATION_RUN; break;		// 走る。
	case PlayerStateID::Bite:  animIndex = anim::ANIMATION_NORMAL_ATTACK; break;		// 通常攻撃。
	case PlayerStateID::DefaultAttack:  animIndex = anim::ANIMATION_SPECIAL_ABILITY; break;	// 特殊能力。
	case PlayerStateID::Landmine:	animIndex = anim::ANIMATION_SPECIAL_ABILITY; break;
	case PlayerStateID::Avoid: animIndex = anim::ANIMATION_AVOID; break;

	default: return; // ないなら処理を返す
	}

	modelRender_.PlayAnimation(animIndex); // Idをもとにそのアニメーションの再生
}



/* ==================================== */
/* セットアップ */
/* ==================================== */

void Player::SetUpTranslateRulu()
{
	// ステート（状態）を登録
	{
		stateMachine_.AddState(PlayerStateID::Idle, new IdleState(this));
		stateMachine_.AddState(PlayerStateID::Walk, new WalkState(this));
		stateMachine_.AddState(PlayerStateID::Run, new RunState(this));
		stateMachine_.AddState(PlayerStateID::Bite, new NormalAttackState(this));
		stateMachine_.AddState(PlayerStateID::DefaultAttack, new SpecialAbilityState(this));
		stateMachine_.AddState(PlayerStateID::Avoid, new UtilityState(this));
		stateMachine_.AddState(PlayerStateID::Dead, new DeathState(this));
	}


	// トランジション（遷移ルール）を登録
	{
		// 優先される条件
		{
			stateMachine_.AddGlobalTransition([this]() { if (IsDead()) { return true; } return false; }, PlayerStateID::Dead); /* HPが0なら他のステート関係なく実行 */
		}

		// 一般ルール 
		{
			/** 待機 */
			{
				// 待機 -> 歩き
				stateMachine_.AddTransition(PlayerStateID::Idle, PlayerStateID::Walk, [this]() {
					return stateMachine_.GetStickLAmount() > 0.01f && !stateMachine_.IsDash();
					});

				// 待機 -> 走り
				stateMachine_.AddTransition(PlayerStateID::Idle, PlayerStateID::Run, [this]() {
					return stateMachine_.GetStickLAmount() > 0.01f && stateMachine_.IsDash();
					});

				// 待機 -> 回避
				stateMachine_.AddTransition(PlayerStateID::Idle, PlayerStateID::Avoid, [this]() {
					if (stateMachine_.IsAvoidRequested()) {
						stateMachine_.ClearAvoidRequest();
						return true;
					}
					return false;
					});

				// 待機 -> 通常攻撃
				stateMachine_.AddTransition(PlayerStateID::Idle, PlayerStateID::Bite, [this]() {
					return stateMachine_.IsActionButtonB();
					});
				// 待機 -> 特殊能力
				stateMachine_.AddTransition(PlayerStateID::Idle, PlayerStateID::DefaultAttack, [this]() {
					return CanSpecialAbility();
					});
			}

			/** 歩き */
			{
				// 歩き -> 待機
				stateMachine_.AddTransition(PlayerStateID::Walk, PlayerStateID::Idle, [this]() {
					return stateMachine_.GetStickLAmount() < 0.01f;
					});

				// 歩き -> 走り
				stateMachine_.AddTransition(PlayerStateID::Walk, PlayerStateID::Run, [this]() {
					return stateMachine_.IsDash();
					});

				// 歩き -> 回避
				stateMachine_.AddTransition(PlayerStateID::Walk, PlayerStateID::Avoid, [this]() {
					if (stateMachine_.IsAvoidRequested()) {
						stateMachine_.ClearAvoidRequest();
						return true;
					}
					return false;
					});

				// 歩き -> 通常攻撃
				stateMachine_.AddTransition(PlayerStateID::Walk, PlayerStateID::Bite, [this]() {
					return stateMachine_.IsActionButtonB();
					});
				// 歩き -> 特殊能力
				stateMachine_.AddTransition(PlayerStateID::Walk, PlayerStateID::DefaultAttack, [this]() {
					return CanSpecialAbility();
					});
			}

			/** 走り */
			{
				// 走り -> 待機
				stateMachine_.AddTransition(PlayerStateID::Run, PlayerStateID::Idle, [this]() {
					return stateMachine_.GetStickLAmount() < 0.01f;
					});

				// 走り -> 歩き
				stateMachine_.AddTransition(PlayerStateID::Run, PlayerStateID::Walk, [this]() {
					return !stateMachine_.IsDash();
					});

				// 待機 -> 回避
				stateMachine_.AddTransition(PlayerStateID::Run, PlayerStateID::Avoid, [this]() {
					if (stateMachine_.IsAvoidRequested()) {
						stateMachine_.ClearAvoidRequest();
						return true;
					}
					return false;
					});

				// 走り -> 通常攻撃
				stateMachine_.AddTransition(PlayerStateID::Run, PlayerStateID::Bite, [this]() {
					return stateMachine_.IsActionButtonB();
					});

				// 走り -> 特殊能力
				stateMachine_.AddTransition(PlayerStateID::Run, PlayerStateID::DefaultAttack, [this]() {
					return CanSpecialAbility();
					});
			}

			/** 通常攻撃 */
			{
				// 通常攻撃 → 待機
				stateMachine_.AddTransition(PlayerStateID::Bite, PlayerStateID::Idle, [this]() {
					auto* currentState = static_cast<NormalAttackState*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsFinished();
					});
				// 通常攻撃 -> 回避
				stateMachine_.AddTransition(PlayerStateID::Bite, PlayerStateID::Avoid, [this]() {
					if (stateMachine_.IsAvoidRequested()) {
						stateMachine_.ClearAvoidRequest();
						return true;
					}
					return false;
					});
			}

			/* 特殊能力 */
			{
				// 特殊能力 → 待機
				stateMachine_.AddTransition(PlayerStateID::DefaultAttack, PlayerStateID::Idle, [this]() {
					auto* currentState = static_cast<SpecialAbilityState*>(stateMachine_.GetCurrentState());
					return currentState && currentState->IsFinished();
					});
				// 特殊能力 -> 回避
				stateMachine_.AddTransition(PlayerStateID::DefaultAttack, PlayerStateID::Avoid, [this]() {
					if (stateMachine_.IsAvoidRequested()) {
						stateMachine_.ClearAvoidRequest();
						return true;
					}
					return false;
					});
			}

			/** 汎用スキル */
			{
				// 回避 → 待機
				stateMachine_.AddTransition(PlayerStateID::Avoid, PlayerStateID::Idle, [this]() {
					auto* currentState = static_cast<UtilityState*>(stateMachine_.GetCurrentState());
					// 回避アクションが終了したら待機に戻る
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


bool Player::CanSpecialAbility()
{
	if (stateMachine_.IsActionButtonY()) {
		PlayerStatus* status = GetStatus()->As<PlayerStatus>();
		if (status->CanExecuteSpecialAbility()) {
			return true;
		}
	}
	return false;
}
/* ==================================== */
/* 更新・描画処理 */
/* ==================================== */
Player::Player()
{
	PlayerStatus* status = new PlayerStatus();
	status_ = status;
}
Player::~Player()
{
	delete status_;
	status_ = nullptr;
}
bool Player::Start()
{
	// ステータスの初期化
	status_->As<PlayerStatus>()->Init(); 

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
			// TODO::Boss都の初期の距離を離すため
			transform_.localPosition = Vector3(300.0f, 0.0f, 0.0f);

			transform_.UpdateTransform();
			modelRender_.SetPosition(transform_.position);
			modelRender_.SetRotation(transform_.rotation);
			modelRender_.SetScale(transform_.scale);
			modelRender_.Update();
		}
	}

	// キャラクターコントローラーの精製
	{
		charaCon_.Init(COLLISION_RADIUS, COLLISION_HEIGHT, transform_.position);
	}

	// ゴーストコリジョンを生成
	{
		damageBody_ = std::make_unique<GhostBody>();
		damageBody_->CreateCapsule(this, CharacterID::PlayerID(), COLLISION_RADIUS, COLLISION_HEIGHT, ghost::CollisionAttribute::PlayerDef, ghost::CollisionAttributeMask::Player);
	
		damageBody_->SetPosition(transform_.position);
	}

	// ステート遷移のルール設定
	SetUpTranslateRulu();

	// 共通処理を呼び出す
	return Character::Start();
}

void Player::Update()
{
	if (!isUpdate_) return;

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

		// キャラコン
		{
			transform_.localPosition = charaCon_.Execute(transform_.position, 1.0f);
		}

		if (transform_.localPosition.y < 0.0f)
		{
			Vector3 pos = transform_.localPosition;
			pos.y = 0.0f;
			transform_.localPosition = pos;
			charaCon_.SetPosition(pos); // キャラコン内部の座標も合わせる
		}
		transform_.UpdateTransform();

		// モデルへ反映
		modelRender_.SetPosition(transform_.position);
		modelRender_.SetRotation(transform_.rotation);
		modelRender_.Update();
	}

	// ステータス
	{
		PlayerStatus* status = GetStatus()->As<PlayerStatus>();
		status->Update();
	}

	// ゴーストコリジョン
	{
		Vector3 collisionPos = transform_.position + Vector3(0.0f, 40.0f, 0.0f);
		damageBody_->SetPosition(collisionPos);
	}
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
	// スキルを作成
	switch (type)
	{
	case NormalAttackType::enBite:
		activeNormalAttack_ = std::make_unique<Bite>();
		break;
	default:
		activeNormalAttack_.reset(); // 何も装備していない状態
		break;
	}

	// プレイヤーステータスに通常攻撃を登録
	const char* skillKey = ToNormalAttackKey(type);
	if (skillKey) {
		status_->As<PlayerStatus>()->EquipSkill("NormalAttack", "NormalAttack", skillKey);
	}
}

void Player::EquipAbility(AbilityType type) 
{
	switch (type) {
	case AbilityType::enLandmine:
		activeAbility_ = std::make_unique<Landmine>();
		break;

	case AbilityType::enFireMagic:
		// activeAbility_ = std::make_unique<FireMagic>();
		break;

	default:
		activeAbility_ = std::make_unique<DefaultAttack>();
		break;
	}

	// プレイヤーステータスに特殊攻撃を登録
	const char* skillKey = ToAbilityKey(type);
	if (skillKey) {
		status_->As<PlayerStatus>()->EquipSkill("SpecialAttack", "SpecialAttack", skillKey);
	}
}

void Player::EquipUtility(UtilityType type) 
{
	switch (type) {
	case UtilityType::enAvoid:
		activeUtility_ = std::make_unique<Avoid>();
		break;
	default:
		activeUtility_ = std::make_unique<Avoid>();
		break;
	}

	// プレイヤーステータスに特殊攻撃を登録
	const char* skillKey = ToUtilityKey(type);
	if (skillKey) {
		status_->As<PlayerStatus>()->EquipSkill("Utility", "Utility", skillKey);
	}
}