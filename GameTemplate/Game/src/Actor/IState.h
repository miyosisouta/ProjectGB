#pragma once
/**
 * Stateの基底クラス
 */
class Character;
class IState : public Noncopyable
{
protected:
	bool isFinished_ = false; //!< 現在のステート処理が終わったか
	std::unique_ptr<TaskSchedulerSystem> taskScheduler_; //!< ステート内で使用するタイマー付きタスクの管理
	std::unique_ptr<GhostBody> attackHitbox_; //!< 攻撃判定用コリジョン

public:
	/** 次の処理に行ってもよいか */
	inline virtual bool IsFinished() const { return isFinished_; }
	/** 他のステートに割り込みキャンセル可能か */
	inline virtual bool IsCancelable() const { return false; }
	/** 怯み(スタガー)に対して免疫か（trueの間に受けた怯み要求は発動せず捨てられる） */
	inline virtual bool IsStaggerImmune() const { return false; }


public:
	/** コンストラクタ */
	IState() {}
	/** デストラクタ */
	virtual ~IState() {}

	/** ステート開始時に呼ぶ */
	virtual void Enter() = 0;
	/** 毎フレーム呼ぶ */
	virtual void Update() = 0;
	/** ステート終了時に呼ぶ */
	virtual void Exit() = 0;
};