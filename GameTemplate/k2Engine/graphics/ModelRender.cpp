#include "k2EnginePreCompile.h"
#include "ModelRender.h"
#include "RenderingEngine.h"
#include "DitherCBData.h"


DitherCBData g_ditherCBData;  // 繧ｰ繝ｭ繝ｼ繝舌Ν螟画焚

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
				// ・ｽ・ｽ・ｽ・ｽ・ｽ_・ｽ・ｽ・ｽ・ｽ・ｽO・ｽG・ｽ・ｽ・ｽW・ｽ・ｽ・ｽ・ｽ・ｽ・ｽo・ｽ^・ｽ・ｽ・ｽ・ｽ
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
			// IBL・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽ・ｽ・ｽX・ｽV・ｽ・ｽ・ｽ黷ｽ・ｽﾌで、PBR・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽ・p・ｽ・ｽ・ｽﾄゑｿｽ・ｽ・ｽ
			// ・ｽt・ｽH・ｽ・ｽ・ｽ[・ｽh・ｽ・ｽ・ｽ・ｽ・ｽ_・ｽ・ｽ・ｽ・ｽ・ｽO・ｽﾌ場合・ｽﾍ、・ｽf・ｽB・ｽX・ｽN・ｽ・ｽ・ｽv・ｽ^・ｽq・ｽ[・ｽv・ｽ・ｽ・ｽﾄ擾ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
			// (IBL・ｽe・ｽN・ｽX・ｽ`・ｽ・ｽ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽﾄゑｿｽ・ｽ・ｽﾌで。)
			MaterialReInitData matReInitData;
			if(m_isEnableInstancingDraw) {
				// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽ鼾・ｿｽﾍ、・ｽg・ｽ・ｽSRV・ｽﾉイ・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽﾝ定す・ｽ・ｽB
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
			// ・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
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
		//・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitInstancingDraw(maxInstance);
		//・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitSkeleton(filePath);
		//・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// ・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽﾏみ抵ｿｽ・ｽ_・ｽo・ｽb・ｽt・ｽ@・ｽﾌ計・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		//・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽI・ｽu・ｽW・ｽF・ｽN・ｽg・ｽ`・ｽ・ｽp・ｽX・ｽﾅ使・ｽp・ｽ・ｽ・ｽ・ｽ驛ゑｿｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitModelOnTranslucent(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		//ZPrepass・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		// InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		//・ｽV・ｽ・ｽ・ｽh・ｽE・ｽ}・ｽb・ｽv・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// ・ｽｽ学・ｽf・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitGeometryDatas(maxInstance);
		if (m_isRaytracingWorld) {
			// ・ｽ・ｽ・ｽC・ｽg・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽh・ｽﾉ追会ｿｽ・ｽB
			g_renderingEngine->AddModelToRaytracingWorld(m_translucentModel);
			m_addRaytracingWorldModel = &m_translucentModel;
		}
		
		// ・ｽe・ｽ夋擾ｿｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽ・ｽ・ｽX・ｽV・ｽ・ｽ・ｽ・ｽB
		UpdateWorldMatrixInModes();

	}

	void ModelRender::InitForwardRendering(ModelInitData& initData)
	{
		//・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitInstancingDraw(1);
		InitSkeleton(initData.m_tkmFilePath);

		// todo ・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽﾏみ抵ｿｽ・ｽ_・ｽo・ｽb・ｽt・ｽ@・ｽﾌ計・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitComputeAnimatoinVertexBuffer(initData.m_tkmFilePath, initData.m_modelUpAxis);

		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		//・ｽ・ｬ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽf・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽﾆに・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		m_forwardRenderModel.Init(initData);
		//ZPrepass・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		//InitModelOnZprepass(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis);
		//・ｽV・ｽ・ｽ・ｽh・ｽE・ｽ}・ｽb・ｽv・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitModelOnShadowMap(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis, false);
		// ・ｽｽ学・ｽf・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitGeometryDatas(1);
		// ・ｽ・ｽ・ｽC・ｽg・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽh・ｽﾉ追会ｿｽ・ｽB
		// g_renderingEngine->AddModelToRaytracingWorld(m_forwardRenderModel);
		// m_addRaytracingWorldModel = &m_forwardRenderModel;
		// ・ｽe・ｽ夋擾ｿｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽ・ｽ・ｽX・ｽV・ｽ・ｽ・ｽ・ｽB
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
		// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitInstancingDraw(maxInstance);
		// ・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitSkeleton(filePath);
		// ・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// ・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽﾏみ抵ｿｽ・ｽ_・ｽo・ｽb・ｽt・ｽ@・ｽﾌ計・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		// GBuffer・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitModelOnRenderGBuffer(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		// ZPrepass・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		// ・ｽV・ｽ・ｽ・ｽh・ｽE・ｽ}・ｽb・ｽv・ｽ`・ｽ・ｽp・ｽﾌ・ｿｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// ・ｽｽ学・ｽf・ｽ[・ｽ^・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
		InitGeometryDatas(maxInstance);
		// ・ｽe・ｽ夋擾ｿｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽ・ｽ・ｽX・ｽV・ｽ・ｽ・ｽ・ｽB
		UpdateWorldMatrixInModes();
		
		if (m_isRaytracingWorld) {
			// ・ｽ・ｽ・ｽC・ｽg・ｽ・ｽ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽh・ｽﾉ追会ｿｽ・ｽB
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
			// ・ｽ・ｽ・ｽ・ｽ・ｽ_・ｽ・ｽ・ｽ・ｽ・ｽO・ｽG・ｽ・ｽ・ｽW・ｽ・ｽ・ｽﾉ登・ｽ^・ｽB
			g_renderingEngine->RegisterGeometryData(&geomData);
			instanceId++;
		}
	}
	void ModelRender::InitSkeleton(const char* filePath)
	{
		//・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽﾇみ搾ｿｽ・ｽﾝ。
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
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽﾌで、
			// ・ｽ・ｽ・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽ・ｽ\・ｽz・ｽ・ｽ・ｽ・ｽB
			// ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽﾌ配・ｽ・ｽﾌ・ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽm・ｽﾛゑｿｽ・ｽ・ｽB
			m_worldMatrixArray = std::make_unique<Matrix[]>(m_maxInstance);
			// ・ｽ・ｽ・ｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽ・ｽGPU・ｽﾉ転・ｽ・ｽ・ｽ・ｽ・ｽ驍ｽ・ｽﾟのス・ｽg・ｽ・ｽ・ｽN・ｽ`・ｽ・ｽ・ｽ[・ｽh・ｽo・ｽb・ｽt・ｽ@・ｽ・ｽ・ｽm・ｽﾛ。
			m_worldMatrixArraySB.Init(
				sizeof(Matrix),
				m_maxInstance,
				nullptr
			);
			m_isEnableInstancingDraw = true;
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽX・ｽﾔ搾ｿｽ・ｽ・ｽ・ｽ辜擾ｿｽ[・ｽ・ｽ・ｽh・ｽs・ｽ・ｽﾌ配・ｽ・ｽﾌイ・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽﾉ変奇ｿｽ・ｽ・ｽ・ｽ・ｽe・ｽ[・ｽu・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽB
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

		// ・ｽ・ｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽ・ｽ・ｽZ・ｽb・ｽg・ｽA・ｽb・ｽv・ｽB
		SetupVertexShaderEntryPointFunc(modelInitData);
		if (m_vsEntryOverride) {
			modelInitData.m_vsEntryPointFunc     = m_vsEntryOverride;
			modelInitData.m_vsSkinEntryPointFunc = m_vsEntryOverride;
		}
		// ・ｽ・ｽ・ｽ_・ｽﾌ趣ｿｽ・ｽO・ｽv・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽB
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;
		if (m_animationClips != nullptr) {
			//・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽw・ｽ閧ｷ・ｽ・ｽB
			modelInitData.m_skeleton = &m_skeleton;		}

		if (isShadowReciever) {
			modelInitData.m_psEntryPointFunc = "PSMainShadowReciever";
		}
		//・ｽ・ｽ・ｽf・ｽ・ｽ・ｽﾌ擾ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽw・ｽ閧ｷ・ｽ・ｽB
		modelInitData.m_modelUpAxis = enModelUpAxis;

		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_SNORM;
		modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (m_isEnableInstancingDraw) {
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽ鼾・ｿｽﾍ、・ｽg・ｽ・ｽSRV・ｽﾉイ・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽﾝ定す・ｽ・ｽB
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

		// ・ｽ・ｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽ・ｽ・ｽZ・ｽb・ｽg・ｽA・ｽb・ｽv・ｽB
		SetupVertexShaderEntryPointFunc(modelInitData);
		// ・ｽ・ｽ・ｽ_・ｽﾌ趣ｿｽ・ｽO・ｽv・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽB
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽw・ｽ閧ｷ・ｽ・ｽB
			modelInitData.m_skeleton = &m_skeleton;
		}

		if (g_renderingEngine->IsSoftShadow()) {
			modelInitData.m_psEntryPointFunc = "PSMainSoftShadow";
		}
		else {
			modelInitData.m_psEntryPointFunc = "PSMainHardShadow";
		}
		//・ｽ・ｽ・ｽf・ｽ・ｽ・ｽﾌ擾ｿｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽw・ｽ閧ｷ・ｽ・ｽB
		modelInitData.m_modelUpAxis = enModelUpAxis;
		modelInitData.m_expandConstantBuffer = &g_renderingEngine->GetDeferredLightingCB();
		modelInitData.m_expandConstantBufferSize = sizeof(g_renderingEngine->GetDeferredLightingCB());
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		modelInitData.m_alphaBlendMode = AlphaBlendMode_Trans;

		int expandSRVNo = 0;
		if (m_isEnableInstancingDraw) {
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽ鼾・ｿｽﾍ、・ｽg・ｽ・ｽSRV・ｽﾉイ・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽﾝ定す・ｽ・ｽB
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
		// ・ｽ・ｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽ・ｽ・ｽZ・ｽb・ｽg・ｽA・ｽb・ｽv・ｽB
		SetupVertexShaderEntryPointFunc(modelInitData);

		// ・ｽ・ｽ・ｽ_・ｽﾌ趣ｿｽ・ｽO・ｽv・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽB
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽw・ｽ閧ｷ・ｽ・ｽB
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
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽ鼾・ｿｽﾍ、・ｽg・ｽ・ｽSRV・ｽﾉイ・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽﾝ定す・ｽ・ｽB
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

		// ・ｽ・ｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾌエ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ[・ｽ|・ｽC・ｽ・ｽ・ｽg・ｽ・ｽ・ｽZ・ｽb・ｽg・ｽA・ｽb・ｽv・ｽB
		SetupVertexShaderEntryPointFunc(modelInitData);
		// ・ｽ・ｽ・ｽ_・ｽﾌ趣ｿｽ・ｽO・ｽv・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽB
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽw・ｽ閧ｷ・ｽ・ｽB
			modelInitData.m_skeleton = &m_skeleton;
		}

		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (m_isEnableInstancingDraw) {
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽ鼾・ｿｽﾍ、・ｽg・ｽ・ｽSRV・ｽﾉイ・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽp・ｽﾌデ・ｽ[・ｽ^・ｽ・ｽﾝ定す・ｽ・ｽB
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}

		m_zprepassModel.Init(modelInitData);
	}
	void ModelRender::UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
	{
		K2_ASSERT(instanceNo < m_maxInstance, "・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽX・ｽﾔ搾ｿｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽﾅゑｿｽ・ｽB");
		if (!m_isEnableInstancingDraw) {
			return;
		}
		Matrix worldMatrix;
		if (m_translucentModel.IsInited()) {
			// ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽf・ｽ・ｽ・ｽ・ｽZPrepass・ｽ・ｽ・ｽf・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽﾄゑｿｽ・ｽﾈゑｿｽ・ｽﾌで、・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽg・ｽ・ｽ・ｽB
			worldMatrix = m_translucentModel.CalcWorldMatrix(pos, rot, scale);
		}
		else {
			worldMatrix = m_zprepassModel.CalcWorldMatrix(pos, rot, scale);
		} 
		// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽX・ｽﾔ搾ｿｽ・ｽ・ｽ・ｽ・ｽs・ｽ・ｽﾌイ・ｽ・ｽ・ｽf・ｽb・ｽN・ｽX・ｽ・ｽ・ｽ謫ｾ・ｽ・ｽ・ｽ・ｽB
		int matrixArrayIndex = m_instanceNoToWorldMatrixArrayIndexTable[instanceNo];
		// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽ・ｽ・ｽs・ｽ・ｽ・ｽB
		m_worldMatrixArray[matrixArrayIndex] = worldMatrix;
		if (m_numInstance == 0) {
			//・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽX・ｽ・ｽ・ｽ・ｽ0・ｽﾌ場合・ｽﾌみア・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽﾖ係・ｽﾌ更・ｽV・ｽ・ｽ・ｽs・ｽ・ｽ・ｽB
			// ・ｽX・ｽP・ｽ・ｽ・ｽg・ｽ・ｽ・ｽ・ｽ・ｽX・ｽV・ｽB
			// ・ｽe・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽX・ｽﾌ・ｿｽ・ｽ[・ｽ・ｽ・ｽh・ｽ・ｽﾔへの変奇ｿｽ・ｽﾍ、
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽX・ｽ・ｽ・ｽﾆに行・ｽ・ｽ・ｽK・ｽv・ｽ・ｽ・ｽ・ｽ・ｽ・ｽﾌで、・ｽ・ｽ・ｽ_・ｽV・ｽF・ｽ[・ｽ_・ｽ[・ｽﾅ行・ｽ・ｽ・ｽB
			// ・ｽﾈので、・ｽP・ｽﾊ行・ｽ・ｽ・ｽn・ｽ・ｽ・ｽﾄ、・ｽ・ｽ・ｽf・ｽ・ｽ・ｽ・ｽﾔでボ・ｽ[・ｽ・ｽ・ｽs・ｽ・ｽ・ｽ・ｽ\・ｽz・ｽ・ｽ・ｽ・ｽB
			m_skeleton.Update(g_matIdentity);
			//・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ・ｽi・ｽﾟゑｿｽB
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

		//・ｽA・ｽj・ｽ・ｽ・ｽ[・ｽV・ｽ・ｽ・ｽ・ｽ・ｽ・ｽi・ｽﾟゑｿｽB
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);

	}
	void ModelRender::Draw(RenderContext& rc)
	{
		if (m_isEnableInstancingDraw) {
			// ・ｽC・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽV・ｽ・ｽ・ｽO・ｽ`・ｽ・ｽﾍビ・ｽ・ｽ・ｽ[・ｽt・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽJ・ｽ・ｽ・ｽ・ｽ・ｽO・ｽﾍ行・ｽ・ｽﾈゑｿｽ・ｽB
			g_renderingEngine->AddRenderObject(this);
			m_worldMatrixArraySB.Update(m_worldMatrixArray.get());
			m_numInstance = 0;
		}
		else {
			// ・ｽﾊ擾ｿｽ`・ｽ・ｽ
			if (m_geometryDatas.empty() || m_geometryDatas[0].IsInViewFrustum()) {
				// ・ｽr・ｽ・ｽ・ｽ[・ｽt・ｽ・ｽ・ｽX・ｽ^・ｽ・ｽ・ｽﾉ含まゑｿｽﾄゑｿｽ・ｽ・ｽB
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
		// ・ｽ・ｽ・ｽ_・ｽﾌ趣ｿｽ・ｽO・ｽv・ｽZ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽ・ｽf・ｽB・ｽX・ｽp・ｽb・ｽ`・ｽ・ｽ・ｽ・ｽB
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
			// alpha縺縺台ｿ晄戟縺励※繧ｰ繝ｭ繝ｼ繝舌Ν縺ｮ蛟､・医き繝｡繝ｩ蠎ｧ讓吶↑縺ｩ・峨ｒ豈弱ヵ繝ｬ繝ｼ繝繧ｳ繝斐・
			float savedAlpha = m_localDitherCBData.ditherAlpha;
			m_localDitherCBData = g_ditherCBData;
			m_localDitherCBData.ditherAlpha = savedAlpha;

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