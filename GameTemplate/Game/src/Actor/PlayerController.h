#pragma once

class Player;

class PlayerController : public IGameObject
{
private:
	Player* m_target = nullptr;


public:
	PlayerController() {};
	~PlayerController() {};

	bool Start() override { return true; };
	void Update() override;
	void Render(RenderContext& rc) override {};

	/** 操作対象の設定 */
	void SetTarget(Player* target)
	{
		m_target = target;
	}

private:
	/** 左スティックの入力量を取得 */
	Vector3 GetStickL();
	/** 左スティックによる方向を使って回転を計算 */
	Quaternion ComputeRotation();
};