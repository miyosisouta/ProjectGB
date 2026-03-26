#pragma once

class Player;
class StateMachine;

class PlayerController : public IGameObject
{
private:
	Player* target_ = nullptr; //!< 操作対象

	bool aButtonHeld_ = false; //!< Aボタンを入力し続けているか
	bool avoidInputConsumed_ = false;
	float aButtonHeldTime_ = 0.0f; //!< 入力し続けている時間

public:
	PlayerController() {};
	~PlayerController() {};

	bool Start() override { return true; };
	void Update() override;
	void Render(RenderContext& rc) override {};

	/** 操作対象の設定 */
	void SetTarget(Player* target)
	{
		target_= target;
	}

private:
	/** 左スティックの入力量を取得 */
	Vector3 GetStickL();
	/** 左スティックによる方向を使って回転を計算 */
	Quaternion ComputeRotation();
	/** Aボタンの入力量 */
	void UpdateAButton(StateMachine* sm);
};