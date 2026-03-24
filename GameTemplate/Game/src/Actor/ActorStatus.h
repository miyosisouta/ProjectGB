#pragma once

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

	/*
	 * 
	 */
	bool CanExecute() const
	{
		return coolTimer <= 0.0f;
	}


	bool IsReadyFrame() const
	{
		return isReadyFrame;
	}
};

class ActorStatus
{
protected:
	int hp_ = 0;
	int maxHp_ = 0;
	bool isTakeDamage = false;	//!< ダメージを受けたフレーム

	int attack_ = 0;
	int defence_ = 0;
	int critical_ = 0;
	float moveSpeed_ = 0.0f;


public:
	ActorStatus() {}
	virtual ~ActorStatus() {}

	virtual void Update()
	{
		isTakeDamage = false;
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
		isTakeDamage = true;
	}
	void Heal(const int value)
	{
		hp_ += value;
		if (hp_ > maxHp_) {
			hp_ = maxHp_;
		}
	}

	bool IsTakeDamage() const { return isTakeDamage; }



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


	virtual void Update() override
	{
		skillNormalAttack.Update();
		skillSpecialAbility.Update();
		skillUtility.Update();

		ActorStatus::Update();
	}


public:
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


	bool IsReadyFrameNormalAttack() const
	{
		return skillNormalAttack.IsReadyFrame();
	}
	bool IsReadyFrameSpecialAbility() const
	{
		return skillSpecialAbility.IsReadyFrame();
	}
	bool IsReadyFrameUtility() const
	{
		return skillUtility.IsReadyFrame();
	}
};




class PlayerStatus : public CharacterStatus
{
public:
	PlayerStatus()
	{
		// TODO: 仮
		hp_ = 5;
		maxHp_ = 5;
	}

	void SetupSkillCoolDown(const int normalAttackCD, const int specialAbilityCD, const int utilityCD)
	{
		skillNormalAttack.coolDownTime = normalAttackCD;
		skillSpecialAbility.coolDownTime = specialAbilityCD;
		skillUtility.coolDownTime = utilityCD;
	}
};




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