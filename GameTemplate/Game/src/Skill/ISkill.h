#pragma once
/**
 * スキルの基底クラス
 */
class Character;
class ISkill : public Noncopyable
{
protected:
	bool isFinished_ = false; //!< 攻撃が終わったかどうか
	bool isCancelable_ = false; //!< 途中で回避を行えるか

public:
	/** 終了状態の設定 */
	inline void SetFinished(const bool flg) { isFinished_ = flg; }
	/** 攻撃が終わったか */
	inline bool IsFinished() { return isFinished_; }
	/** キャンセル可否の設定 */
	inline void SetisCancelable(const bool flg) { isCancelable_ = flg; }
	/** 途中で回避を行えるか */
	inline bool IsCancelable() { return isCancelable_; }

public:
	/** コンストラクタ */
	ISkill() {}
	/** デストラクタ */
	virtual ~ISkill() {}

	/** スキル開始時に呼ぶ */
	virtual void Enter(Character* p) = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update(Character* p) = 0;
	/** スキル終了時に呼ぶ */
	virtual void Exit(Character* p) = 0;
};