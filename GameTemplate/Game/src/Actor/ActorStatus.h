#pragma once

struct CoolDown
{
public:
	float coolTimer = 0.0f;		//!< クールタイムの時間計算用
	float coolDownTime = 0.0f;	//!< クールタイムの時間設定用


public:
	/* 時間を経過させる */
	void Update()
	{
		coolTimer -= g_gameTime->GetFrameDeltaTime();
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

	/*
	 * 
	 */
	bool CanExecute() const
	{
		return coolTimer <= 0.0f;
	}
};

class ActorStatus
{
protected:
	int hp_ = 0;
	int maxHp_ = 0;

	int attack_ = 0;
	int defence_ = 0;
	int critical_ = 0;
	float moveSpeed_ = 0.0f;


public:
	ActorStatus() {}
	virtual ~ActorStatus() {}


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
	}
	void Heal(const int value)
	{
		hp_ += value;
		if (hp_ > maxHp_) {
			hp_ = maxHp_;
		}
	}


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




class CharacterStatus : public ActorStatus
{
protected:
	CoolDown skillNormalAttack;
	CoolDown skillSpecialAbility;
	CoolDown skillUtility;


public:
	CharacterStatus() {}
	virtual ~CharacterStatus() {}


	virtual void Update()
	{
		skillNormalAttack.Update();
		skillSpecialAbility.Update();
		skillUtility.Update();
	}


public:
	void SetupSkillCoolDown(const int normalAttackCD, const int specialAbilityCD, const int utilityCD)
	{
		skillNormalAttack.coolDownTime = normalAttackCD;
		skillSpecialAbility.coolDownTime = specialAbilityCD;
		skillUtility.coolDownTime = utilityCD;
	}


	void ExecuteNormalAttack()
	{
		skillNormalAttack.Execute();
	}
	void ExecuteSpecialAbility()
	{
		skillSpecialAbility.Execute();
	}
	void ExecuteUtility()
	{
		skillUtility.Execute();
	}


	bool CanExecuteNormalAttack() const
	{
		return skillNormalAttack.CanExecute();
	}
	bool CanExecuteSpecialAbility() const
	{
		return skillSpecialAbility.CanExecute();
	}
	bool CanExecuteUtility() const
	{
		return skillUtility.CanExecute();
	}
};




class PlayerStatus : public CharacterStatus
{
};




class BossStatus : public CharacterStatus
{
protected:
	CoolDown skillBossAttack_;
	

public:
	BossStatus() {}
	virtual ~BossStatus() {}

public:
	void Update() override 
	{
		skillBossAttack_.Update();
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