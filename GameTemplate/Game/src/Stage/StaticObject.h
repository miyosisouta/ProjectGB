/**
 * StaticObject.h
 * 動かないオブジェクト
 */


#pragma once
class StaticObject
{
private:
	/** 当たり判定用のモデル */
	ModelRender model_;
	Transform transform_;

public:
	ModelRender* GetModel(){ return &model_; }
	inline Transform* GetTransform() { return &transform_; }

	void Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal);

public:
	StaticObject();
	~StaticObject();
	
	void Render(RenderContext& rc);
};