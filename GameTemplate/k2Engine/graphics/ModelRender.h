#pragma once

#include "MyRenderer.h"
#include "geometry/AABB.h"
#include "geometry/geometryData.h"
#include "graphics/ComputeAnimationVertexBuffer.h"
#include "graphics/DitherCBData.h"

namespace nsK2Engine {
	class RenderingEngine;

	/** スプラットマップシェーダー専用の定数バッファデータ (b2)。ディザリングとは独立。 */
	struct SplatColorCBData
	{
		float param0     = 1.0f; //!< テクスチャ0 (草) 明るさ倍率
		float param1     = 1.0f; //!< テクスチャ1 (岩土) 明るさ倍率
		float param2     = 1.0f; //!< テクスチャ2 (腐葉土) 明るさ倍率
		float param3     = 1.0f; //!< グローバルHSV明るさ倍率
		float saturation = 1.0f; //!< 彩度倍率 (1.0=変化なし, 1.5=鮮やか, 0.5=くすみ)
		float _pad0 = 0.0f;
		float _pad1 = 0.0f;
		float _pad2 = 0.0f;
	};

	/** スキンモデルレンダラー */
	class ModelRender : public IRenderer
	{
	public:
		ModelRender();
		~ModelRender();

		/** 初期化。通常はこの関数で初期化する。maxInstance>1でインスタンシング描画になる。isFrontCullingOnDrawShadowMapは地面など薄いモデルではfalseにする。 */
		void Init(
			const char* filePath,
			AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnModelUpAxis enModelUpAxis = enModelUpAxisZ,
			bool isShadowReciever = true,
			int maxInstance = 1,
			bool isFrontCullingOnDrawShadowMap = false);

		/** 半透明描画を行うオブジェクトを初期化する。 */
		void IniTranslucent(
			const char* filePath,
			AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnModelUpAxis enModelUpAxis = enModelUpAxisZ,
			bool isShadowReciever = true,
			int maxInstance = 1,
			bool isFrontCullingOnDrawShadowMap = false);

		/** 独自シェーディングを行いたい場合の初期化。 */
		void InitForwardRendering(ModelInitData& initData);

		/** 更新処理。 */
		void Update();

		/** インスタンシングデータの更新。 */
		void UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale);

		/** 描画処理。 */
		void Draw(RenderContext& rc);

		/** アニメーション再生。interpolateTimeは補間時間（秒）。 */
		void PlayAnimation(int animNo, float interpolateTime = 0.0f)
		{
			m_animation.Play(animNo, interpolateTime);
		}

		/** アニメーションの再生中か？ */
		bool IsPlayingAnimation() const
		{
			return m_animation.IsPlaying();
		}

		/** モデルを取得。 */
		Model& GetModel()
		{
			if (m_renderToGBufferModel.IsInited()) {
				return m_renderToGBufferModel;
			}
			else if (m_forwardRenderModel.IsInited()) {
				return m_forwardRenderModel;
			}
			else if (m_translucentModel.IsInited())
			{
				return m_translucentModel;
			}
			// ここまで来るのはおかしい
			return m_zprepassModel;
		}

		/** 座標・回転・拡縮を一括設定。インスタンシング描画が有効の場合は無効。 */
		void SetTRS(const Vector3& pos, const Quaternion& rotation, const Vector3& scale)
		{
			SetPosition(pos);
			SetRotation(rotation);
			SetScale(scale);
		}

		/** 座標を設定。インスタンシング描画が有効の場合は無効。 */
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
		}

		/** 座標を設定（x,y,z個別指定）。 */
		void SetPosition(float x, float y, float z)
		{
			SetPosition({ x, y, z });
		}

		/** 回転を設定。インスタンシング描画が有効の場合は無効。 */
		void SetRotation(const Quaternion& rotation)
		{
			m_rotation = rotation;
		}

		/** 拡大率を設定。インスタンシング描画が有効の場合は無効。 */
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
		}
		void SetScale(float x, float y, float z)
		{
			SetScale({ x, y, z });
		}

		/** シャドウキャスターフラグを設定する。 */
		void SetShadowCasterFlag(bool flag)
		{
			m_isShadowCaster = flag;
		}

		/** アニメーション再生速度を設定する（標準=1.0）。 */
		void SetAnimationSpeed(const float animationSpeed)
		{
			m_animationSpeed = animationSpeed;
		}

		/** ボーン名からボーン番号を取得する。存在しない場合は-1を返す。 */
		int FindBoneID(const wchar_t* boneName) const
		{
			return m_skeleton.FindBoneID(boneName);
		}

		/** ボーン番号からボーンを取得する。 */
		Bone* GetBone(int boneNo) const
		{
			return m_skeleton.GetBone(boneNo);
		}

		void AddAnimationEvent(AnimationEventListener eventListener)
		{
			m_animation.AddAnimationEventListener(eventListener);
		}

		/** シャドウキャスターか？ */
		bool IsShadowCaster() const
		{
			return m_isShadowCaster;
		}

		/** インスタンス数を取得。 */
		int GetNumInstance() const
		{
			return m_numInstance;
		}

		/** インスタンシング描画を行うか？ */
		bool IsInstancingDraw() const
		{
			return m_isEnableInstancingDraw;
		}

		/** ワールド行列を取得。インスタンシング無効の場合は instanceId 無視。 */
		const Matrix& GetWorldMatrix(int instanceId) const
		{
			if (IsInstancingDraw()) {
				return m_worldMatrixArray[instanceId];
			}
			return m_zprepassModel.GetWorldMatrix();
		}

		/** インスタンスを削除する。シーンから除外したい場合に利用する。 */
		void RemoveInstance(int instanceNo);

		/** レイトレースワールドに登録するかを設定する。専用モデル（天など）はInit前にfalseで呼ぶ。 */
		void SetRaytracingWorld(bool flag) {
			m_isRaytracingWorld = flag;
		}

		/** 透明度ディザ値を設定する（0.0=完全透明, 1.0=不透明）。GBufferパスでのみ有効。 */
		void SetDitherAlpha(float alpha)
		{
			m_localDitherCBData.ditherAlpha = alpha;
		}

		/** 透明度ディザ値を取得する。 */
		float GetDitherAlpha() const
		{
			return m_localDitherCBData.ditherAlpha;
		}

		/** GBufferパスで使うカスタムシェーダーのパスを設定する（Init()呼び出し前に設定すること）。 */
		void SetGBufferFxOverride(const char* fxPath) { m_gBufferFxOverride = fxPath; }

		/** GBufferパスの頂点シェーダーエントリーポイントを上書きする（Init()呼び出し前に設定すること）。 */
		void SetVSEntryOverride(const char* entry) { m_vsEntryOverride = entry; }

		/** GBufferパスに追加のStructuredBuffer SRVを渡す。index=0→t11, 1→t12 ...（Init()前に設定）。 */
		void SetExtraGBufferSRV(int index, StructuredBuffer* sb)
		{
			K2_ASSERT(index >= 0 && index < 4, "SetExtraGBufferSRV: index out of range");
			m_extraGBufferSRVs[index] = sb;
		}

		/** GBufferパスに追加のテクスチャSRVを渡す。index=0→t11, 1→t12 ...（Init()前に設定）。SetExtraGBufferSRVと同スロット共有。 */
		void SetExtraGBufferTextureSRV(int index, Texture* tex)
		{
			K2_ASSERT(index >= 0 && index < 4, "SetExtraGBufferTextureSRV: index out of range");
			m_extraGBufferSRVs[index] = tex;
		}

		/** スプラットシェーダー専用カラーパラメータを設定する (b2)。p0=草,p1=岩土,p2=腐葉土の明るさ倍率, p3=全体HSV明るさ。デフォルト1.0。 */
		void SetSplatColorParams(float p0, float p1, float p2, float p3)
		{
			m_splatColorCBData.param0 = p0;
			m_splatColorCBData.param1 = p1;
			m_splatColorCBData.param2 = p2;
			m_splatColorCBData.param3 = p3;
		}

		/** スプラットシェーダーの彩度を設定する。1.0=変化なし, 1.5=鮮やか, 0.5=くすみ。 */
		void SetSplatSaturation(float saturation)
		{
			m_splatColorCBData.saturation = saturation;
		}

	private:
		/** スケルトンの初期化。 */
		void InitSkeleton(const char* filePath);

		/** アニメーションの初期化。 */
		void InitAnimation(AnimationClip* animationClips,
			int numAnimationClips,
			EnModelUpAxis enModelUpAxis);

		/** 頂点計算パスから呼び出される処理。 */
		void OnComputeVertex(RenderContext& rc);

		/** シャドウマップへの描画パスから呼ばれる処理。 */
		void OnRenderShadowMap(
			RenderContext& rc,
			int ligNo,
			int shadowMapNo,
			const Matrix& lvpMatrix) override;

		/** ZPrepassから呼ばれる処理。 */
		void OnZPrepass(RenderContext& rc) override;

		/** G-Buffer描画パスから呼ばれる処理。 */
		void OnRenderToGBuffer(RenderContext& rc) override;

		/** フォワードレンダーパスから呼ばれる処理。 */
		void OnForwardRender(RenderContext& rc) override;

		/** 半透明オブジェクト描画パスから呼ばれる処理。 */
		void OnTlanslucentRender(RenderContext& rc) override;

		/** 各モデルのワールド行列を更新する。 */
		void UpdateWorldMatrixInModes();

	private:
		/** レンダリングエンジンのイベントを受け取った時に呼ばれる処理。 */
		void OnRecievedEventFromRenderingEngine(RenderingEngine::EnEvent enEvent);

		/** シャドウマップ描画用のモデルを初期化。 */
		void InitModelOnShadowMap(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis modelUpAxis,
			bool isFrontCullingOnDrawShadowMap
		);

		/** ZPrepass描画用のモデルを初期化。 */
		void InitModelOnZprepass(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis modelUpAxis
		);

		/** インスタンシング描画用の初期化処理を実行。 */
		void InitInstancingDraw(int maxInstance);

		/** アニメーション済み頂点バッファの計算処理を初期化する。 */
		void InitComputeAnimatoinVertexBuffer(
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis);

		/** GBuffer描画用のモデルを初期化。 */
		void InitModelOnRenderGBuffer(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis,
			bool isShadowReciever);

		/** 半透明オブジェクト描画パスで使用するモデルを初期化する。 */
		void InitModelOnTranslucent(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis,
			bool isShadowReciever
		);

		/** 各モデルの頂点シェーダーのエントリーポイントを設定。 */
		void SetupVertexShaderEntryPointFunc(ModelInitData& modelInitData);

		/** モデルのバウンディングボリュームを初期化する（AABB構築）。 */
		void InitBoundingVolume();

		/** 幾何学データの初期化。 */
		void InitGeometryDatas(int maxInstance);

	public:
		static const int NUM_SHADOW_LIGHT = 1;

	private:
		AnimationClip*               m_animationClips = nullptr;                                    //!< アニメーションクリップ
		int                          m_numAnimationClips = 0;                                       //!< アニメーションクリップの数
		Vector3                      m_position = Vector3::Zero;                                    //!< 座標
		Quaternion                   m_rotation = Quaternion::Identity;                             //!< 回転
		Vector3                      m_scale = Vector3::One;                                        //!< 拡大率
		EnModelUpAxis                m_enFbxUpAxis = enModelUpAxisZ;                                //!< FBXの上方向
		Animation                    m_animation;                                                   //!< アニメーション
		ComputeAnimationVertexBuffer m_computeAnimationVertexBuffer;                                //!< アニメーション済み頂点バッファの計算処理
		Model*                       m_addRaytracingWorldModel = nullptr;                           //!< レイトレースワールドに登録するモデル
		Model                        m_zprepassModel;                                               //!< ZPrepassで描画されるモデル
		Model                        m_forwardRenderModel;                                          //!< フォワードレンダリングの描画パスで描画されるモデル
		Model                        m_translucentModel;                                            //!< 半透明モデル
		Model                        m_renderToGBufferModel;                                        //!< RenderToGBufferで描画されるモデル
		Model                        m_shadowModels[MAX_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];         //!< シャドウマップに描画するモデル
		ConstantBuffer               m_drawShadowMapCameraParamCB[MAX_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP]; //!< シャドウマップ作成時に必要なカメラパラメータ用の定数バッファ
		bool                         m_isUpdateAnimation = true;                                    //!< アニメーションを更新するか？
		Skeleton                     m_skeleton;                                                    //!< スケルトン
		bool                         m_isShadowCaster = true;                                      //!< シャドウキャスターフラグ
		float                        m_animationSpeed = 1.0f;                                      //!< アニメーション再生速度
		int                          m_numInstance = 0;                                             //!< インスタンスの数
		int                          m_maxInstance = 1;                                             //!< 最大インスタンス数
		bool                         m_isEnableInstancingDraw = false;                              //!< インスタンシング描画が有効か？
		bool                         m_isRaytracingWorld = true;                                    //!< レイトレースワールドに登録するか？
		std::unique_ptr<Matrix[]>    m_worldMatrixArray;                                            //!< ワールド行列の配列
		StructuredBuffer             m_worldMatrixArraySB;                                          //!< ワールド行列の配列のストラクチャードバッファ
		std::vector<GemometryData>   m_geometryDatas;                                               //!< ジオメトリデータ
		std::unique_ptr<int[]>       m_instanceNoToWorldMatrixArrayIndexTable;                      //!< インスタンス番号からワールド行列配列インデックスへの変換テーブル
		DitherCBData                 m_localDitherCBData;                                           //!< per-objectのディザCB（ditherAlphaはセッターで変更）
		const char*                  m_gBufferFxOverride = nullptr;                                 //!< GBufferパスのカスタムFXファイルパス
		const char*                  m_vsEntryOverride   = nullptr;                                 //!< GBufferパスのVSエントリー上書き
		IShaderResource*             m_extraGBufferSRVs[4] = {};                                    //!< GBufferパスに追加するSRV（t11〜t14）。StructuredBuffer*/Texture* 両対応
		SplatColorCBData             m_splatColorCBData;                                            //!< スプラット専用CB (b2)。ディザCBとは独立

	};
}
