#pragma once
/**
 * GrassObject.h
 * 草オブジェクト専用クラス
 */

class GrassObject
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
	const Transform* GetTransform() const { return &transform_; }
	/** 描画設定 */
	void SetDraw(const bool isDraw) { isDraw_ = isDraw; }
	/** ディザーアルファの設定 (0.0=完全透明, 1.0=不透明) */
	void SetDitherAlpha(float alpha) { model_.SetDitherAlpha(alpha); }
	/** ディザーアルファの取得 */
	float GetDitherAlpha() const { return model_.GetDitherAlpha(); }

	/** 初期化 */
	void Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal);


public:
	GrassObject();
	~GrassObject();
	
	void Update();
	void Render(RenderContext& rc);
};