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
	Transform transform_; //!< トランスフォーム

private:
	std::string name_; //!< オブジェクト名
	bool isDraw_ = true; //!< 描画するか
    float ditherAlpha_ = 1.0f; //!< ディザリングの透明度

	/** スプラットシェーダー設定 (Init() 前に SetupSplatShader() でセットする) */
	struct SplatShaderConfig {
		Texture* splatMap   = nullptr; //!< RGBスプラットマップ
		Texture* kusaTex    = nullptr; //!< 草テクスチャ
		Texture* tuthiTex   = nullptr; //!< 岩土テクスチャ
		Texture* fuyoudoTex = nullptr; //!< 腐葉土テクスチャ
		/** スプラットマップが設定済みか */
		inline bool isValid() const { return splatMap != nullptr; }
	};
	SplatShaderConfig splatConfig_; //!< スプラットシェーダー設定

public:
	/** モデルの取得 */
	inline ModelRender* GetModel() { return &model_; }
	/** トランスフォームの取得 */
	inline Transform* GetTransform() { return &transform_; }
	/** オブジェクトの名前を設定 */
	inline void SetName(const std::string name) { name_ = name; }
	/** オブジェクトの名前を取得 */
	inline std::string GetName() const { return name_; }
	/** 描画状態の設定 */
	inline void SetDraw(const bool isDraw) { isDraw_ = isDraw; }
	/** 描画状態の取得 */
	inline bool GetDraw()const { return isDraw_; }
	/** ディザーアルファの設定 (0.0=完全透明, 1.0=不透明) */
	inline void SetDitherAlpha(float alpha) { model_.SetDitherAlpha(alpha); }
	/** ディザーアルファの取得 */
	inline float GetDitherAlpha() const { return model_.GetDitherAlpha(); }
	/** スプラットシェーダー用カラーパラメータを設定する。p0=草,p1=岩土,p2=腐葉土の明るさ倍率, p3=全体HSV明るさ倍率。デフォルトはすべて1.0。 */
	inline void SetSplatColorParams(float p0, float p1, float p2, float p3) { model_.SetSplatColorParams(p0, p1, p2, p3); }
	/** スプラットシェーダーの彩度を設定する。1.0=変化なし, 1.5=鮮やか, 0.5=くすみ。 */
	inline void SetSplatSaturation(float saturation) { model_.SetSplatSaturation(saturation); }

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
	/** コンストラクタ */
	StaticObject();
	/** デストラクタ */
	~StaticObject();

	/** 描画処理 */
	void Render(RenderContext& rc);
};