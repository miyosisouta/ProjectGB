#pragma once


class ActorStatus;


class Actor : public IGameObject
{
public:
	/** 例外としてpublic */
	Transform transform_; //!< トランスフォーム

protected:
	ModelRender modelRender_; //!< モデルレンダー

	ActorStatus* status_ = nullptr; //!< ステータス


public:
	/* コンストラクタ */
	Actor() {}
	/* デストラクタ */
	~Actor() {}

	/* スタート処理 */
	virtual bool Start() override;
	/* 更新処理 */
	virtual void Update() override;
	/* 描画処理 */
	virtual void Render(RenderContext& rc) override;


public:
	/** グローバルな座標を取得 */
	inline const Vector3 GetTransformPosition() const { return transform_.position; }
	/** グローバルな回転を取得 */
	inline const Quaternion GetTransformRotation() const { return transform_.rotation; }
	/** グローバルな大きさを取得 */
	inline const Vector3 GetTransformScale() const { return transform_.scale; }

	/* ステータスを取得 */
	ActorStatus* GetStatus() { return status_; }
};