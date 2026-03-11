#pragma once


class Actor : public IGameObject
{
public:
	/** 例外としてpublic */
	app::math::Transform transform_; //!< トランスフォーム

protected:
	ModelRender modelRender_; //!< モデルレンダー


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
};