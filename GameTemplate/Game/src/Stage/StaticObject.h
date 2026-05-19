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
	std::string name_; // オブジェクト名
	bool isDraw_ = true; //!< 描画するか
    float ditherAlpha_ = 1.0f; //!< ディザリングの透明度

	// スプラットシェーダー設定 (Init() 前に SetupSplatShader() でセットする)
	struct SplatShaderConfig {
		Texture* splatMap   = nullptr;
		Texture* kusaTex    = nullptr;
		Texture* tuthiTex   = nullptr;
		Texture* fuyoudoTex = nullptr;
		bool isValid() const { return splatMap != nullptr; }
	};
	SplatShaderConfig splatConfig_;

public:
	/** モデルの取得 */
	ModelRender* GetModel() { return &model_; }
	/** トランスフォームの取得 */
	inline Transform* GetTransform() { return &transform_; }
	/** オブジェクトの名前を設定 */
	void SetName(const std::string name) { name_ = name; }
	/** オブジェクトの名前を取得 */
	std::string GetName() const { return name_; }
	/** 描画状態の設定 */
	void SetDraw(const bool isDraw) { isDraw_ = isDraw; }
	/** 描画状態の取得 */
	bool GetDraw()const { return isDraw_; }
	/** ディザーアルファの設定 (0.0=完全透明, 1.0=不透明) */
	void SetDitherAlpha(float alpha) { model_.SetDitherAlpha(alpha); }
	/** ディザーアルファの取得 */
	float GetDitherAlpha() const { return model_.GetDitherAlpha(); }
	/** スプラットシェーダー用カラーパラメータを設定する。p0=草,p1=岩土,p2=腐葉土の明るさ倍率, p3=全体HSV明るさ倍率。デフォルトはすべて1.0。 */
	void SetSplatColorParams(float p0, float p1, float p2, float p3) { model_.SetSplatColorParams(p0, p1, p2, p3); }
	/** スプラットシェーダーの彩度を設定する。1.0=変化なし, 1.5=鮮やか, 0.5=くすみ。 */
	void SetSplatSaturation(float saturation) { model_.SetSplatSaturation(saturation); }

	/**
	 * RGBスプラットマップによる地面テクスチャブレンドを有効にする。
	 * 必ず Init() より前に呼び出すこと。
	 * @param splatMap   RGBスプラットマップ (R=草, G=岩土, B=腐葉土)
	 * @param kusaTex    草テクスチャ
	 * @param tuthiTex   岩土テクスチャ
	 * @param fuyoudoTex 腐葉土テクスチャ
	 */
	void SetupSplatShader(Texture* splatMap, Texture* kusaTex, Texture* tuthiTex, Texture* fuyoudoTex);

	/** 初期化 */
	void Init(const char* path, const Vector3& pos, const Quaternion& rot, const Vector3& scal);


public:
	StaticObject();
	~StaticObject();

	void Render(RenderContext& rc);
};