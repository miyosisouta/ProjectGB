#pragma once

class Player;
class StateMachine;

class PlayerController : public IGameObject
{
private:
	Player* target_ = nullptr; //!< 操作対象


private:
	/** 左スティックの入力量を取得 */
	Vector3 GetStickL();
	/** 左スティックによる方向を使って回転を計算 */
	Quaternion ComputeRotation();
	/** Aボタンの入力量 */
	void UpdateAButton(StateMachine* sm);

public:
	/** 操作対象の設定 */
	inline void SetTarget(Player* target)
	{
		target_ = target;
	}

public:
	/** コンストラクタ */
	PlayerController() {};
	/** デストラクタ */
	~PlayerController() {};

	/** スタート処理 */
	bool Start() override { return true; };
	/** 更新処理 */
	void Update() override;
	/** 描画処理 */
	void Render(RenderContext& rc) override {};
};