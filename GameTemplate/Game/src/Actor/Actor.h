#pragma once


class Actor : public IGameObject
{
public:
	/** 例外としてpublic */
	app::math::Transform m_transform; //!< トランスフォーム

protected:
	ModelRender m_modelRender; //!< モデルレンダー


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