#pragma once
#include "src/Core/ParameterManager.h"

/*
 * クールダウン構造体
 */
struct CoolDown
{
public:
	float coolTimer = 0.0f;		//!< クールタイムの時間計算用
	float coolDownTime = 0.0f;	//!< クールタイムの時間設定用
	bool isReadyFrame = false;	//!< クールダウン終わった1フレームだけtrue

public:
	/* 時間を経過させる */
	void Update()
	{
		isReadyFrame = false;
		if (coolTimer > 0.0f) {
			coolTimer -= g_gameTime->GetFrameDeltaTime();
		}
		if (coolTimer < 0.0f) {
			isReadyFrame = true;
			coolTimer = 0.0f;
		}
	}

	/* 
	 * クールタイムを発生させる
	 * coolTimerに時間を入れることでクールタイムが発生
	 */
	void Execute()
	{
		coolTimer = coolDownTime;
	}

	/* 
	 * 残り時間を取得 
	 * UIにて使う
	 */
	float GetCoolDownTimer() const
	{ 
		return coolTimer;
	}

	/** 実行できるか */
	bool CanExecute() const
	{
		return coolTimer <= 0.0f;
	}

	/* フレーム準備ができているか */
	bool IsReadyFrame() const
	{
		return isReadyFrame;
	}
};


/* ===================================================== */
/** 実体のあるオブジェクトのステータスクラス */
/* ===================================================== */

class ActorStatus
{
protected:
	int hp_ = 0;
	int maxHp_ = 0;
	bool isTakeDamage_ = false;	//!< ダメージを受けたフレーム

	int attack_ = 0;
	int defence_ = 0;
	int critical_ = 0;
	float moveSpeed_ = 0.0f;


public:
	ActorStatus() {}
	virtual ~ActorStatus() {}

	virtual void Update()
	{
		isTakeDamage_ = false;
	}


public:
	int GetHP() const { return hp_; }
	int GetMaxHP() const { return maxHp_; }

	int GetAttack() const { return attack_; }
	int GetDefence() const { return defence_; }
	int GetCritical() const { return critical_; }
	float GetMoveSpeed() const { return moveSpeed_; }

	bool IsDead() const { return hp_ <= 0; }


public:
	void Damage(const int damage)
	{
		hp_ -= damage;
		if (hp_ < 0) {
			hp_ = 0;
		}
		isTakeDamage_ = true;
	}
	void Heal(const int value)
	{
		hp_ += value;
		if (hp_ > maxHp_) {
			hp_ = maxHp_;
		}
	}

	bool IsTakeDamage() const { return isTakeDamage_; }



public:
	template <typename T>
	bool Is() const
	{
		T* ptr = dynamic_cast<T*>(this);
		return ptr != nullptr;
	}

	template <typename T>
	T* As()
	{
		return dynamic_cast<T*>(this);
	}
};


/* ===================================================== */
/** キャラクターのステータス */
/* ===================================================== */
class CharacterStatus : public ActorStatus
{
public:
	CharacterStatus() {}
	virtual ~CharacterStatus() {}

	virtual void Update() override { ActorStatus::Update(); }
};


/* ===================================================== */
/* プレイヤー用のステータス */
/* ===================================================== */

class PlayerStatus : public CharacterStatus
{
public:
	enum class InvincibleFlags : uint32_t
	{
		enNone   = 0,
		enAvoid  = 1 << 1,	//!< 回避中
		enDamage = 1 << 2,	//!< ダメージを受けたとき
		enSkill  = 1 << 3,	//!< 攻撃中
	};

public:
	// --- 無敵フラグ操作 ---

	// 指定した無敵フラグをONにする
	void AddInvincible(InvincibleFlags flag)
	{
		invincibleFlag_ |= static_cast<uint32_t>(flag);
	}

	// 指定した無敵フラグをOFFにする
	void RemoveInvincible(InvincibleFlags flag)
	{
		invincibleFlag_ &= ~static_cast<uint32_t>(flag);
	}

	// 指定したフラグが今ONになっているか確認する
	bool HasInvincible(InvincibleFlags flag) const
	{
		return (invincibleFlag_ & static_cast<uint32_t>(flag)) != 0;
	}

	// いずれかの無敵フラグが1つでも立っているか確認する
	bool IsInvincible() const { return invincibleFlag_ != 0; }


private:
	uint8_t invincibleFlag_ = 0;

	CoolDown skillNormalAttack_;
	CoolDown skillSpecialAbility_;
	CoolDown skillUtility_;

public:
	PlayerStatus() {}

	/// <summary>
	/// ParameterManager からプレイヤーのステータスとスキルCDを取得して初期化する
	/// ParameterManager::CreateInstance() と各Load関数を呼んだ後に使うこと
	/// </summary>
	/// <param name="characterKey">CharacterStatusData.json の key (例: "Player")</param>
	/// <param name="normalAttackKey">PlayerSkillStatus.json NormalAttack の key (例: "Bite")</param>
	/// <param name="specialAbilityKey">PlayerSkillStatus.json SpecialAttack の key (例: "DefaultAttack")</param>
	/// <param name="utilityKey">PlayerSkillStatus.json Utility の key (例: "Dodge")</param>
	void Init(
		const std::string& characterKey    = "Player",
		const std::string& normalAttackKey = "Bite",
		const std::string& specialAbilityKey = "DefaultAttack",
		const std::string& utilityKey      = "Dodge")
	{
		auto& pm = ParameterManager::Get();

		// --- キャラクターのベースステータスを取得 ---
		const auto* chara = pm.GetCharacterStatus(characterKey);
		if (chara)
		{
			hp_       = chara->hp;
			maxHp_    = chara->hp;
			attack_   = chara->attack;
			critical_ = chara->criticalRate;
		}

		// --- スキルのクールダウンを取得 ---
		// NormalAttack カテゴリの先頭スキル
		const auto* normalSkill = pm.GetPlayerSkill("NormalAttack", normalAttackKey);
		if (normalSkill)
		{
			skillNormalAttack_.coolDownTime = normalSkill->cooldown;
		}

		// SpecialAttack カテゴリの先頭スキル
		const auto* specialSkill = pm.GetPlayerSkill("SpecialAttack", specialAbilityKey);
		if (specialSkill)
		{
			skillSpecialAbility_.coolDownTime = specialSkill->cooldown;
		}

		// Utility カテゴリの先頭スキル
		const auto* utilitySkill = pm.GetPlayerSkill("Utility", utilityKey);
		if (utilitySkill)
		{
			skillUtility_.coolDownTime = utilitySkill->cooldown;
		}
	}

	void Update() override
	{
		skillNormalAttack_.Update();
		skillSpecialAbility_.Update();
		skillUtility_.Update();

		CharacterStatus::Update();
	}

public:
	/** 通常攻撃を実行する */
	void ExecuteNormalAttack()    { skillNormalAttack_.Execute(); }
	/** スペシャルアビリティを実行する */
	void ExecuteSpecialAbility()  { skillSpecialAbility_.Execute(); }
	/** ユーティリティスキルを実行する */
	void ExecuteUtility()         { skillUtility_.Execute(); }

	/** 通常攻撃が使用可能か */
	bool CanExecuteNormalAttack()    const { return skillNormalAttack_.CanExecute(); }
	/** スペシャルアビリティが使用可能か */
	bool CanExecuteSpecialAbility()  const { return skillSpecialAbility_.CanExecute(); }
	/** ユーティリティスキルが使用可能か */
	bool CanExecuteUtility()         const { return skillUtility_.CanExecute(); }

	/** 通常攻撃のクールダウンが明けた1フレーム */
	bool IsReadyFrameNormalAttack()   const { return skillNormalAttack_.IsReadyFrame(); }
	/** スペシャルアビリティのクールダウンが明けた1フレーム */
	bool IsReadyFrameSpecialAbility() const { return skillSpecialAbility_.IsReadyFrame(); }
	/** ユーティリティスキルのクールダウンが明けた1フレーム */
	bool IsReadyFrameUtility()        const { return skillUtility_.IsReadyFrame(); }

	/** 通常攻撃の残りクールダウン時間 (UI用) */
	float GetNormalAttackCoolTimer()   const { return skillNormalAttack_.GetCoolDownTimer(); }
	/** スペシャルアビリティの残りクールダウン時間 (UI用) */
	float GetSpecialAbilityCoolTimer() const { return skillSpecialAbility_.GetCoolDownTimer(); }
	/** ユーティリティスキルの残りクールダウン時間 (UI用) */
	float GetUtilityCoolTimer()        const { return skillUtility_.GetCoolDownTimer(); }
};


/* ===================================================== */
/* ボス用のステータス */
/* ===================================================== */

class BossStatus : public CharacterStatus
{
protected:
	CoolDown skillBossAttack_;

public:
	BossStatus() {}
	virtual ~BossStatus() {}

	/// <summary>
	/// ParameterManager からボスのステータスとスキルCDを取得して初期化する
	/// ParameterManager::CreateInstance() と各Load関数を呼んだ後に使うこと
	/// </summary>
	/// <param name="characterKey">CharacterStatusData.json の key (例: "Gorilla")</param>
	/// <param name="skillKey">BossSkillStatus.json のスキルkey (例: "NormalAttack")</param>
	void Init(
		const std::string& characterKey,
		const std::string& skillKey = "NormalAttack")
	{
		auto& pm = ParameterManager::Get();

		// --- キャラクターのベースステータスを取得 ---
		const auto* chara = pm.GetCharacterStatus(characterKey);
		if (chara)
		{
			hp_     = chara->hp;
			maxHp_  = chara->hp;
			attack_ = chara->attack;
		}

		// --- スキルのクールダウンを取得 ---
		// BossSkillStatus.json では category=ボス名、key=スキル名
		const auto* skill = pm.GetBossSkill(characterKey, skillKey);
		if (skill)
		{
			skillBossAttack_.coolDownTime = static_cast<float>(skill->attack); 
			// NOTE: BossSkillStatus.json に cooldown フィールドを追加した場合は
			//       skill->cooldown に差し替えてください
		}
	}

public:
	void Update() override
	{
		skillBossAttack_.Update();
		CharacterStatus::Update();
	}

	/** ボス攻撃を実行する */
	void ExecuteBossAttack()           { skillBossAttack_.Execute(); }
	/** ボス攻撃が使用可能か */
	bool CanExecuteBossAttack()  const { return skillBossAttack_.CanExecute(); }
	/** ボス攻撃CDが明けた1フレーム */
	bool IsReadyFrameBossAttack() const { return skillBossAttack_.IsReadyFrame(); }
};


/* ===================================================== */
/* スキル用のステータス */
/* ===================================================== */
