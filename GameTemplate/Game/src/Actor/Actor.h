#pragma once


class Actor : public IGameObject
{
public:
	/** 例外としてpublic */
	app::math::Transform m_transform;

protected:
	ModelRender m_modelRender;


public:
	Actor() {}
	~Actor() {}

	virtual bool Start() override;
	virtual void Update() override;
	virtual void Render(RenderContext& rc) override;
};