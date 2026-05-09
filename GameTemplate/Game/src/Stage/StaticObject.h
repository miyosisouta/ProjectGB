#pragma once
/**
 * StaticObject.h
 * 動かないオブジェクト
 */

class StaticObject
{
private:
	/** 当たり判定用のモデル */
	ModelRender model_;
	Transform transform_;

private:
	bool isDraw_ = false; //!< 描画するか
    float ditherAlpha_ = 1.0f; //!< ディザリングの透明度

public:
	/** モデルの取得 */
	ModelRender* GetModel() { return &model_; }
	/** トランスフォームの取得 */
	inline Transform* GetTransform() { return &transform_; }
	/** 描画設定 */
	void SetDraw(const bool isDraw) { isDraw_ = isDraw; }
	/** ディザーアルファの設定 (0.0=完全透明, 1.0=不透明) */
	void SetDitherAlpha(float alpha) { model_.SetDitherAlpha(alpha); }
	/** ディザーアルファの取得 */
	float GetDitherAlpha() const { return model_.GetDitherAlpha(); }

	/** 初期化 */
	void Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal);


public:
	StaticObject();
	~StaticObject();
	
	void Render(RenderContext& rc);
};