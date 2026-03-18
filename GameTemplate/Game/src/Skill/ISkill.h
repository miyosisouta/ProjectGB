#pragma once
/*
 * Stateの基底クラス
 */
class Character;
class ISkill : public Noncopyable
{	
protected:
	bool isFinished_ = false; //!< 攻撃が終わったかどうか
	bool isCancelable_ = false; //!< 途中で回避を行えるか

public:
	inline void SetFinished(const bool flg) { isFinished_ = flg; }
	inline bool IsFinished() { return isFinished_; }
	inline void SetisCancelable(const bool flg) { isCancelable_ = flg; }
	inline bool IsCancelable() { return isCancelable_; }				

public:
	ISkill() {}
	virtual ~ISkill() {}

	virtual void Enter(Character* p) = 0;
	virtual void Update(Character* p) = 0;
	virtual void Exit(Character* p) = 0;
};