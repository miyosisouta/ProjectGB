#pragma once

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
	float GetCoolDownTimer() 
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


	virtual void Update() override{ ActorStatus::Update(); }
};


/* ===================================================== */
/* プレイヤー用のステータス */
/* ===================================================== */

class PlayerStatus : public CharacterStatus
{
public:
	enum class InvincibleFlags : uint32_t
	{
		enNone = 0,
		enAvoid = 1 << 1,		//!< 回避中
		enDamage = 1 << 2,	//!< ダメージを受けたとき
		enSkill = 1 << 3,		//!< 攻撃中
	};

public:
	// --- 無敵フラグ操作 ---

	// 指定した無敵フラグをONにする
	// 例: 回避開始時に Dodge ビットを立てる
	void AddInvincible(InvincibleFlags flag)
	{
		invincibleFlag_ |= static_cast<uint32_t>(flag);
	}

	// 指定した無敵フラグをOFFにする
	// 例: 無敵時間終了・Exit()の強制解除
	void RemoveInvincible(InvincibleFlags flag)
	{
		invincibleFlag_ &= ~static_cast<uint32_t>(flag);
	}

	// 指定したフラグが今ONになっているか確認する
	// 例: 回避無敵だけを個別に確認したいとき
	bool HasInvincible(InvincibleFlags flag) const
	{
		return (invincibleFlag_ & static_cast<uint32_t>(flag)) != 0;
	}

	// いずれかの無敵フラグが1つでも立っているか確認する
	// 例: 当たり判定でダメージを受けるか判断するとき
	bool IsInvincible() const { return invincibleFlag_ != 0; }


private:
	uint8_t invincibleFlag_ = 0;

	CoolDown skillNormalAttack;
	CoolDown skillSpecialAbility;
	CoolDown skillUtility;

public:
	PlayerStatus()
	{
		// TODO: 仮
		hp_ = 5;
		maxHp_ = 5;
	}

	void Update() override
	{
		skillNormalAttack.Update();
		skillSpecialAbility.Update();
		skillUtility.Update();

		CharacterStatus::Update();
	}

	void SetupSkillCoolDown(const int normalAttackCD, const int specialAbilityCD, const int utilityCD)
	{
		skillNormalAttack.coolDownTime = normalAttackCD;
		skillSpecialAbility.coolDownTime = specialAbilityCD;
		skillUtility.coolDownTime = utilityCD;
	}

public:
	/** 通常攻撃を実行する */
	void ExecuteNormalAttack()
	{
		skillNormalAttack.Execute();
	}

	/** スキルを実行する */
	void ExecuteSpecialAbility()
	{
		skillSpecialAbility.Execute();
	}

	/** 汎用スキルを実行する */
	void ExecuteUtility()
	{
		skillUtility.Execute();
	}

	/* 通常攻撃が使用可能か */
	bool CanExecuteNormalAttack() const
	{
		return skillNormalAttack.CanExecute();
	}
	/** スキルが使用可能か */
	bool CanExecuteSpecialAbility() const
	{
		return skillSpecialAbility.CanExecute();
	}

	/** 汎用スキルが使用可能か */
	bool CanExecuteUtility() const
	{
		return skillUtility.CanExecute();
	}

	/** 通常攻撃が可能か */
	bool IsReadyFrameNormalAttack() const
	{
		return skillNormalAttack.IsReadyFrame();
	}

	/** スキル攻撃が可能か */
	bool IsReadyFrameSpecialAbility() const
	{
		return skillSpecialAbility.IsReadyFrame();
	}

	/** 汎用スキルが可能か */
	bool IsReadyFrameUtility() const
	{
		return skillUtility.IsReadyFrame();
	}
};


/* ===================================================== */
/* ボス用のステータス */
/* ===================================================== */

class BossStatus : public CharacterStatus
{
protected:
	CoolDown skillBossAttack_;
	
public:
	void InitStatus(int maxHp, int attack) 
	{
		maxHp_ = maxHp;
		attack_ = attack;
	}

public:
	BossStatus() 
	{
		hp_ = 5;
		maxHp_ = 5;
	}
	virtual ~BossStatus() {}

public:
	void Update() override 
	{
		skillBossAttack_.Update();

		CharacterStatus::Update();
	}

	void SetupSkillBossAttackCoolDown(const int bossAttackCD)
	{
		skillBossAttack_.coolDownTime = bossAttackCD;
	}


	void ExecuteBossAttack()
	{
		skillBossAttack_.Execute();
	}

	bool CanExecuteBossAttack_() const
	{
		return skillBossAttack_.CanExecute();
	}
};


/* ===================================================== */
/* スキル用のステータス */
/* ===================================================== */

