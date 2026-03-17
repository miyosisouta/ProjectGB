#pragma once
#include "Character.h"


class State;
class Player : public Character
{
private:
	std::unique_ptr<GhostBody> playerHitbox_; //!< プレイヤーの当たり判定用コリジョン
	Vector3 moveVelocity_ = Vector3::Zero; //!< 毎フレームの移動速度を保持する変数

public:
	inline void SetMoveVelocity(const Vector3& velocity) { moveVelocity_ = velocity; } //!< 移動速度の設定
	inline Vector3 GetMoveVelocity() { return moveVelocity_; } //!< 移動速度を取得

public:
	/** 遷移ルールのセットアップ */
	void SetUpTranslateRulu();

	/** 
	 * アニメーションの再生。
	 * id : ステートのIDによってアニメーションのインデックスを決める
	 */
	void PlayAnimation(StateID id);


public:
	/* コンストラクタ */
	Player();
	/* デストラクタ */
	~Player() {}

	/* スタート処理 */
	virtual bool Start() override;
	/* 更新処理 */
	virtual void Update() override;
	/* 描画処理 */
	virtual void Render(RenderContext& rc) override;
};