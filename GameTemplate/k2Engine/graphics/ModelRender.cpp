#include "k2EnginePreCompile.h"
#include "ModelRender.h"
#include "RenderingEngine.h"
#include "DitherCBData.h"


DitherCBData g_ditherCBData;  // グローバル変数

namespace nsK2Engine {
	ModelRender::ModelRender()
	{
		g_renderingEngine->AddEventListener(
			this,
			[&](RenderingEngine::EnEvent enEvent) { OnRecievedEventFromRenderingEngine(enEvent); }
		);
	}
	ModelRender::~ModelRender()
	{
		if (g_renderingEngine != nullptr) {
			g_renderingEngine->RemoveEventListener(this);
			for (auto& geomData : m_geometryDatas) {
				// ジオメトリデータをレンダリングエンジンから登録解除する
				g_renderingEngine->UnregisterGeometryData(&geomData);
			}
			if (m_addRaytracingWorldModel) {
				g_renderingEngine->RemoveModelFromRaytracingWorld(*m_addRaytracingWorldModel);
			}
		}
	}
	void ModelRender::OnRecievedEventFromRenderingEngine(RenderingEngine::EnEvent enEvent)
	{
		if (enEvent == RenderingEngine::enEventReInitIBLTexture
			&& m_translucentModel.IsInited()
		) {
			// IBLテクスチャが再初期化されたので、PBRシェーダーを使っているフォワードレンダリングの場合は
			// ディスクリプタヒープを作り直す必要がある。
			// (IBLテクスチャを使っているため。)
			MaterialReInitData matReInitData;
			if(m_isEnableInstancingDraw) {
				// インスタンシング描画が有効な場合は、追加SRVにインスタンシング描画用のデータを設定する。
				matReInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
			}
			matReInitData.m_expandShaderResoruceView[1] = &g_renderingEngine->GetIBLTexture();
			m_translucentModel.ReInitMaterials(matReInitData);
		}
	}
	void ModelRender::SetupVertexShaderEntryPointFunc(ModelInitData& modelInitData)
	{
		modelInitData.m_vsSkinEntryPointFunc = "VSMainUsePreComputedVertexBuffer";
		modelInitData.m_vsEntryPointFunc = "VSMainUsePreComputedVertexBuffer";

		if (m_animationClips != nullptr) {
			// アニメーションを使う場合。
			modelInitData.m_vsSkinEntryPointFunc = "VSMainSkinUsePreComputedVertexBuffer";
		}
	}
	void ModelRender::IniTranslucent(
		const char* filePath,
		AnimationClip* animationClips,
		int numAnimationClips,
		EnModelUpAxis enModelUpAxis,
		bool isShadowReciever,
		int maxInstance,
		bool isFrontCullingOnDrawShadowMap)
	{
		// インスタンシング描画用のデータを初期化する。
		InitInstancingDraw(maxInstance);
		// スケルトンを初期化する。
		InitSkeleton(filePath);
		// アニメーションを初期化する。
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// アニメーション済み頂点バッファの計算処理を初期化する。
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		// 半透明オブジェクト描画パスで使用するモデルを初期化する。
		InitModelOnTranslucent(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		//ZPrepass描画用のモデルを初期化する。
		// InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		//シャドウマップ描画用のモデルを初期化する。
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// 幾何学データを初期化する。
		InitGeometryDatas(maxInstance);
		if (m_isRaytracingWorld) {
			// レイトレースワールドに追加。
			g_renderingEngine->AddModelToRaytracingWorld(m_translucentModel);
			m_addRaytracingWorldModel = &m_translucentModel;
		}

		// ワールド行列を更新する。
		UpdateWorldMatrixInModes();

	}

	void ModelRender::InitForwardRendering(ModelInitData& initData)
	{
		// インスタンシング描画用のデータを初期化する。
		InitInstancingDraw(1);
		InitSkeleton(initData.m_tkmFilePath);

		// todo アニメーション済み頂点バッファの計算処理を初期化する。
		InitComputeAnimatoinVertexBuffer(initData.m_tkmFilePath, initData.m_modelUpAxis);

		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		// 共通バッファフォーマットにデータを格納する際にモデルを初期化する。
		m_forwardRenderModel.Init(initData);
		//ZPrepass描画用のモデルを初期化する。
		//InitModelOnZprepass(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis);
		//シャドウマップ描画用のモデルを初期化する。
		InitModelOnShadowMap(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis, false);
		// 幾何学データを初期化する。
		InitGeometryDatas(1);
		// レイトレースワールドに追加。
		// g_renderingEngine->AddModelToRaytracingWorld(m_forwardRenderModel);
		// m_addRaytracingWorldModel = &m_forwardRenderModel;
		// ワールド行列を更新する。
		UpdateWorldMatrixInModes();
	}

	void ModelRender::Init(const char* filePath,
		AnimationClip* animationClips,
		int numAnimationClips,
		EnModelUpAxis enModelUpAxis,
		bool isShadowReciever,
		int maxInstance,
		bool isFrontCullingOnDrawShadowMap)
	{
		// インスタンシング描画用のデータを初期化する。
		InitInstancingDraw(maxInstance);
		// スケルトンを初期化する。
		InitSkeleton(filePath);
		// アニメーションを初期化する。
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// アニメーション済み頂点バッファの計算処理を初期化する。
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		// GBuffer描画用のモデルを初期化する。
		InitModelOnRenderGBuffer(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		// ZPrepass描画用のモデルを初期化する。
		InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		// シャドウマップ描画用のモデルを初期化する。
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// 幾何学データを初期化する。
		InitGeometryDatas(maxInstance);
		// ワールド行列を更新する。
		UpdateWorldMatrixInModes();

		if (m_isRaytracingWorld) {
			// レイトレースワールドに追加。
			g_renderingEngine->AddModelToRaytracingWorld(m_renderToGBufferModel);
			m_addRaytracingWorldModel = &m_renderToGBufferModel;
		}
	}

	void ModelRender::InitGeometryDatas(int maxInstance)
	{
		m_geometryDatas.resize(maxInstance);
		int instanceId = 0;
		for (auto& geomData : m_geometryDatas) {
			geomData.Init(this, instanceId);
			// レンダリングエンジンに登録する。
			g_renderingEngine->RegisterGeometryData(&geomData);
			instanceId++;
		}
	}
	void ModelRender::InitSkeleton(const char* filePath)
	{
		// スケルトンのデータを読み込む。
		std::string skeletonFilePath = filePath;
		int pos = (int)skeletonFilePath.find(".tkm");
		skeletonFilePath.replace(pos, 4, ".tks");
		m_skeleton.Init(skeletonFilePath.c_str());
	}

	void ModelRender::InitAnimation(AnimationClip* animationClips, int numAnimationClips, EnModelUpAxis enModelUpAxis)
	{
		m_animationClips = animationClips;
		m_numAnimationClips = numAnimationClips;
		if (m_animationClips != nullptr) {
			m_animation.Init(m_skeleton,
				m_animationClips,
				numAnimationClips);
		}
	}

	void ModelRender::InitInstancingDraw(int maxInstance)
	{
		m_maxInstance = maxInstance;
		if (m_maxInstance > 1) {
			// インスタンシング描画を行うので、共通データをデータ構造で初期化する。
			// ワールド行列の配列はあらかじめメモリを確保する。
			m_worldMatrixArray = std::make_unique<Matrix[]>(m_maxInstance);
			// ワールド行列をGPUに転送するためのストラクチャードバッファを確保。
			m_worldMatrixArraySB.Init(
				sizeof(Matrix),
				m_maxInstance,
				nullptr
			);
			m_isEnableInstancingDraw = true;
			// インスタンス番号からワールド行列の配列のインデックスに変換するテーブルを初期化する。
			m_instanceNoToWorldMatrixArrayIndexTable = std::make_unique<int[]>(m_maxInstance);
			for (int instanceNo = 0; instanceNo < m_maxInstance; instanceNo++) {
				m_instanceNoToWorldMatrixArrayIndexTable[instanceNo] = instanceNo;
			}
		}
	}

	void ModelRender::InitModelOnRenderGBuffer(
		RenderingEngine& renderingEngine,
		const char* tkmFilePath,
		EnModelUpAxis enModelUpAxis,
		bool isShadowReciever
	)
	{
		ModelInitData modelInitData;
		modelInitData.m_fxFilePath = m_gBufferFxOverride
			? m_gBufferFxOverride
			: "Assets/shader/preProcess/RenderToGBufferFor3DModel.fx";

		m_localDitherCBData = g_ditherCBData;
		modelInitData.m_expandConstantBuffer = &m_localDitherCBData;
		modelInitData.m_expandConstantBufferSize = sizeof(DitherCBData);
		modelInitData.m_expandConstantBuffer2 = &m_splatColorCBData;
		modelInitData.m_expandConstantBufferSize2 = sizeof(SplatColorCBData);

		// 頂点シェーダーのエントリーポイントをセットアップする。
		SetupVertexShaderEntryPointFunc(modelInitData);
		if (m_vsEntryOverride) {
			modelInitData.m_vsEntryPointFunc     = m_vsEntryOverride;
			modelInitData.m_vsSkinEntryPointFunc = m_vsEntryOverride;
		}
		// 頂点の事前計算済みバッファを使う。
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;
		if (m_animationClips != nullptr) {
			// スケルトンを指定する。
			modelInitData.m_skeleton = &m_skeleton;		}

		if (isShadowReciever) {
			modelInitData.m_psEntryPointFunc = "PSMainShadowReciever";
		}
		// モデルの上方向を指定する。
		modelInitData.m_modelUpAxis = enModelUpAxis;

		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_SNORM;
		modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (m_isEnableInstancingDraw) {
			// インスタンシング描画が有効な場合は、追加SRVにインスタンシング描画用のデータを設定する。
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}
		// extra SRVs (slot1=t11, ...) for custom shader
		for (int i = 0; i < 4; i++) {
			if (m_extraGBufferSRVs[i]) {
				modelInitData.m_expandShaderResoruceView[1 + i] = m_extraGBufferSRVs[i];
			}
		}
		m_renderToGBufferModel.Init(modelInitData);

	}
	void ModelRender::InitComputeAnimatoinVertexBuffer(
		const char* tkmFilePath,
		EnModelUpAxis enModelUpAxis)
	{
		StructuredBuffer* worldMatrxiArraySB = nullptr;
		if (m_isEnableInstancingDraw) {
			worldMatrxiArraySB = &m_worldMatrixArraySB;
		}

		m_computeAnimationVertexBuffer.Init(
			tkmFilePath,
			m_skeleton.GetNumBones(),
			m_skeleton.GetBoneMatricesTopAddress(),
			enModelUpAxis,
			m_maxInstance,
			worldMatrxiArraySB
		);
	}
	void ModelRender::InitModelOnTranslucent(
		RenderingEngine& renderingEngine,
		const char* tkmFilePath,
		EnModelUpAxis enModelUpAxis,
		bool isShadowReciever
	)
	{
		ModelInitData modelInitData;
		modelInitData.m_fxFilePath = "Assets/shader/model.fx";

		// 頂点シェーダーのエントリーポイントをセットアップする。
		SetupVertexShaderEntryPointFunc(modelInitData);
		// 頂点の事前計算済みバッファを使う。
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			// スケルトンを指定する。
			modelInitData.m_skeleton = &m_skeleton;
		}

		if (g_renderingEngine->IsSoftShadow()) {
			modelInitData.m_psEntryPointFunc = "PSMainSoftShadow";
		}
		else {
			modelInitData.m_psEntryPointFunc = "PSMainHardShadow";
		}
		// モデルの上方向を指定する。
		modelInitData.m_modelUpAxis = enModelUpAxis;
		modelInitData.m_expandConstantBuffer = &g_renderingEngine->GetDeferredLightingCB();
		modelInitData.m_expandConstantBufferSize = sizeof(g_renderingEngine->GetDeferredLightingCB());
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		modelInitData.m_alphaBlendMode = AlphaBlendMode_Trans;

		int expandSRVNo = 0;
		if (m_isEnableInstancingDraw) {
			// インスタンシング描画が有効な場合は、追加SRVにインスタンシング描画用のデータを設定する。
			modelInitData.m_expandShaderResoruceView[expandSRVNo] = &m_worldMatrixArraySB;
		}
		expandSRVNo++;
		modelInitData.m_expandShaderResoruceView[expandSRVNo] = &g_renderingEngine->GetIBLTexture();
		expandSRVNo++;
		g_renderingEngine->QueryShadowMapTexture([&](Texture& shadowMap) {
			modelInitData.m_expandShaderResoruceView[expandSRVNo] = &shadowMap;
			expandSRVNo++;
		});
		m_translucentModel.Init(modelInitData);
	}

	void ModelRender::InitModelOnShadowMap(
		RenderingEngine& renderingEngine,
		const char* tkmFilePath,
		EnModelUpAxis modelUpAxis,
		bool isFrontCullingOnDrawShadowMap
	)
	{
		ModelInitData modelInitData;
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_modelUpAxis = modelUpAxis;
		modelInitData.m_cullMode = isFrontCullingOnDrawShadowMap ? D3D12_CULL_MODE_FRONT : D3D12_CULL_MODE_BACK;
		// 頂点シェーダーのエントリーポイントをセットアップする。
		SetupVertexShaderEntryPointFunc(modelInitData);

		// 頂点の事前計算済みバッファを使う。
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			// スケルトンを指定する。
			modelInitData.m_skeleton = &m_skeleton;
		}

		modelInitData.m_fxFilePath = "Assets/shader/preProcess/DrawShadowMap.fx";
		if (g_renderingEngine->IsSoftShadow()) {
			modelInitData.m_colorBufferFormat[0] = g_softShadowMapFormat.colorBufferFormat;
		}
		else {
			modelInitData.m_colorBufferFormat[0] = g_hardShadowMapFormat.colorBufferFormat;
		}

		if (m_isEnableInstancingDraw) {
			// インスタンシング描画が有効な場合は、追加SRVにインスタンシング描画用のデータを設定する。
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}

		for (int ligNo = 0; ligNo < /*MAX_DIRECTIONAL_LIGHT*/NUM_SHADOW_LIGHT; ligNo++)
		{
			ConstantBuffer* cameraParamCBArray = m_drawShadowMapCameraParamCB[ligNo];
			Model* shadowModelArray = m_shadowModels[ligNo];
			for (int shadowMapNo = 0; shadowMapNo < NUM_SHADOW_MAP; shadowMapNo++) {
				cameraParamCBArray[shadowMapNo].Init(sizeof(Vector4), nullptr);
				modelInitData.m_expandConstantBuffer = &cameraParamCBArray[shadowMapNo];
				modelInitData.m_expandConstantBufferSize = sizeof(Vector4);
				shadowModelArray[shadowMapNo].Init(modelInitData);
			}
		}
	}

	void ModelRender::InitModelOnZprepass(
		RenderingEngine& renderingEngine,
		const char* tkmFilePath,
		EnModelUpAxis modelUpAxis
	)
	{
		ModelInitData modelInitData;
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_fxFilePath = "Assets/shader/preProcess/ZPrepass.fx";
		modelInitData.m_modelUpAxis = modelUpAxis;

		// 頂点シェーダーのエントリーポイントをセットアップする。
		SetupVertexShaderEntryPointFunc(modelInitData);
		// 頂点の事前計算済みバッファを使う。
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			// スケルトンを指定する。
			modelInitData.m_skeleton = &m_skeleton;
		}

		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (m_isEnableInstancingDraw) {
			// インスタンシング描画が有効な場合は、追加SRVにインスタンシング描画用のデータを設定する。
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}

		m_zprepassModel.Init(modelInitData);
	}
	void ModelRender::UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
	{
		K2_ASSERT(instanceNo < m_maxInstance, "インスタンス番号が不正です。");
		if (!m_isEnableInstancingDraw) {
			return;
		}
		Matrix worldMatrix;
		if (m_translucentModel.IsInited()) {
			// 半透明モデルとZPrepassモデルは初期化が分かれているため、半透明モデルを使う。
			worldMatrix = m_translucentModel.CalcWorldMatrix(pos, rot, scale);
		}
		else {
			worldMatrix = m_zprepassModel.CalcWorldMatrix(pos, rot, scale);
		}
		// インスタンス番号からワールド行列の配列のインデックスを取得する。
		int matrixArrayIndex = m_instanceNoToWorldMatrixArrayIndexTable[instanceNo];
		// インスタンシング描画を実行する。
		m_worldMatrixArray[matrixArrayIndex] = worldMatrix;
		if (m_numInstance == 0) {
			// インスタンス数が0の場合のみアニメーション関係の更新を行う。
			// スケルトンを更新する。
			// 各インスタンスのローカルからワールドへの変換は、インスタンスごとに行う必要があるので、
			// 頂点シェーダーで行う。なので、単一の行列で初期化して、モデル内でボーン行列を構築できる。
			m_skeleton.Update(g_matIdentity);
			// アニメーションを進める。
			m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);
		}
		m_numInstance++;
	}
	void ModelRender::UpdateWorldMatrixInModes()
	{
		m_zprepassModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		if (m_renderToGBufferModel.IsInited()) {
			m_renderToGBufferModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}
		if (m_forwardRenderModel.IsInited()) {
			m_forwardRenderModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}
		if (m_translucentModel.IsInited()) {
			m_translucentModel.UpdateWorldMatrix(m_position, m_rotation, m_scale);
		}
		for (auto& models : m_shadowModels) {
			for (auto& model : models) {
				if (model.IsInited()) {
					model.UpdateWorldMatrix(m_position, m_rotation, m_scale);
				}
			}
		}
	}
	void ModelRender::Update()
	{
		if (m_isEnableInstancingDraw) {
			return;
		}

		UpdateWorldMatrixInModes();

		if (m_skeleton.IsInited()) {
			m_skeleton.Update(m_zprepassModel.GetWorldMatrix());
		}

		// アニメーションを進める。
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);

	}
	void ModelRender::Draw(RenderContext& rc)
	{
		if (m_isEnableInstancingDraw) {
			// インスタンシング描画はビューフラスタムカリングは行わない。
			g_renderingEngine->AddRenderObject(this);
			m_worldMatrixArraySB.Update(m_worldMatrixArray.get());
			m_numInstance = 0;
		}
		else {
			// 通常描画
			if (m_geometryDatas.empty() || m_geometryDatas[0].IsInViewFrustum()) {
				// ビューフラスタムに含まれている。
				g_renderingEngine->AddRenderObject(this);
			}
		}
	}

	void ModelRender::RemoveInstance(int instanceNo)
	{
		int matrixIndex = m_instanceNoToWorldMatrixArrayIndexTable[instanceNo];

		m_worldMatrixArray[matrixIndex] = g_matZero;
	}
	void ModelRender::OnComputeVertex(RenderContext& rc)
	{
		// 頂点の事前計算済みバッファをディスパッチする。
		if (m_isEnableInstancingDraw) {
			m_computeAnimationVertexBuffer.Dispatch(rc, m_zprepassModel.GetWorldMatrix(), m_maxInstance);
		}
		else {
			m_computeAnimationVertexBuffer.Dispatch(rc, m_zprepassModel.GetWorldMatrix(), 1);
		}
	}
	void ModelRender::OnRenderShadowMap(
		RenderContext& rc,
		int ligNo,
		int shadowMapNo,
		const Matrix& lvpMatrix)
	{

		if (m_isShadowCaster
			&& m_shadowModels[ligNo][shadowMapNo].IsInited()
		) {
			Vector4 cameraParam;
			cameraParam.x = g_camera3D->GetNear();
			cameraParam.y = g_camera3D->GetFar();
			m_drawShadowMapCameraParamCB[ligNo][shadowMapNo].CopyToVRAM(cameraParam);
			m_shadowModels[ligNo][shadowMapNo].Draw(
				rc,
				g_matIdentity,
				lvpMatrix,
				1
			);
		}
	}

	void ModelRender::OnZPrepass(RenderContext& rc)
	{
		m_zprepassModel.Draw(rc, 1);
	}
	void ModelRender::OnRenderToGBuffer(RenderContext& rc)
	{
		if (m_renderToGBufferModel.IsInited()) {
			// per-object値を保持してグローバルの値(カメラ座標など)を毎フレームコピー
			float savedAlpha = m_localDitherCBData.ditherAlpha;
			m_localDitherCBData = g_ditherCBData;
			m_localDitherCBData.ditherAlpha = savedAlpha;
			// m_splatColorCBData は SetSplatColorParams() で直接更新されるため毎フレームコピー不要

			m_renderToGBufferModel.Draw(rc, 1);
		}
	}
	void ModelRender::OnForwardRender(RenderContext& rc)
	{
		if (m_forwardRenderModel.IsInited()) {
			m_forwardRenderModel.Draw(rc, 1);
		}
	}
	void ModelRender::OnTlanslucentRender(RenderContext& rc)
	{
		if (m_translucentModel.IsInited()) {
			m_translucentModel.Draw(rc, 1);
		}
	}
}
