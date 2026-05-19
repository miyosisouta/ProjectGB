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
				// �E��E��E��E��E�_�E��E��E��E��E�O�E�G�E��E��E�W�E��E��E��E��E��E�o�E�^�E��E��E��E�
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
			// IBL�E�e�E�N�E�X�E�`�E��E��E��E��E�X�E�V�E��E��E�ꂽ�E�̂ŁAPBR�E�V�E�F�E�[�E�_�E�[�E��Ep�E��E��E�Ă��E��E�
			// �E�t�E�H�E��E��E�[�E�h�E��E��E��E��E�_�E��E��E��E��E�O�E�̏ꍇ�E�́A�E�f�E�B�E�X�E�N�E��E��E�v�E�^�E�q�E�[�E�v�E��E��E�ď��E��E��E��E��E��E��E��E�B
			// (IBL�E�e�E�N�E�X�E�`�E��E��E��E��E�g�E��E��E�Ă��E��E�̂ŁB)
			MaterialReInitData matReInitData;
			if(m_isEnableInstancingDraw) {
				// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�ꍁE��́A�E�g�E��E�SRV�E�ɃC�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E�ݒ肷�E��E�B
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
			// �E�A�E�j�E��E��E�[�E�V�E��E��E��E��E��E��E��E�B
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
		//�E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E��E��E��E��E��E��E��E�B
		InitInstancingDraw(maxInstance);
		//�E�X�E�P�E��E��E�g�E��E��E��E��E��E��E��E��E��E��E�B
		InitSkeleton(filePath);
		//�E�A�E�j�E��E��E�[�E�V�E��E��E��E��E��E��E��E��E��E��E��E��E�B
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// �E�A�E�j�E��E��E�[�E�V�E��E��E��E��E�ςݒ��E�_�E�o�E�b�E�t�E�@�E�̌v�E�Z�E��E��E��E��E��E��E��E��E��E��E��E��E�B
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		//�E��E��E��E��E��E��E�I�E�u�E�W�E�F�E�N�E�g�E�`�E��E�p�E�X�E�Ŏg�E�p�E��E��E��E�郂�f�E��E��E��E��E��E��E��E��E��E��E�B
		InitModelOnTranslucent(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		//ZPrepass�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		// InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		//�E�V�E��E��E�h�E�E�E�}�E�b�E�v�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// �E�􉽊w�E�f�E�[�E�^�E��E��E��E��E��E��E��E��E�B
		InitGeometryDatas(maxInstance);
		if (m_isRaytracingWorld) {
			// �E��E��E�C�E�g�E��E��E��E��E�[�E��E��E�h�E�ɒǉ��E�B
			g_renderingEngine->AddModelToRaytracingWorld(m_translucentModel);
			m_addRaytracingWorldModel = &m_translucentModel;
		}
		
		// �E�e�E�����[�E��E��E�h�E�s�E��E��E��E�X�E�V�E��E��E��E�B
		UpdateWorldMatrixInModes();

	}

	void ModelRender::InitForwardRendering(ModelInitData& initData)
	{
		//�E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E��E��E��E��E��E��E��E�B
		InitInstancingDraw(1);
		InitSkeleton(initData.m_tkmFilePath);

		// todo �E�A�E�j�E��E��E�[�E�V�E��E��E��E��E�ςݒ��E�_�E�o�E�b�E�t�E�@�E�̌v�E�Z�E��E��E��E��E��E��E��E��E��E��E��E��E�B
		InitComputeAnimatoinVertexBuffer(initData.m_tkmFilePath, initData.m_modelUpAxis);

		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		//�E��E��E��E��E��E��E��E��E��E��E��E��E�f�E�[�E�^�E��E��E��E��E�ƂɁE���E�f�E��E��E��E��E��E��E��E��E��E��E��E��E��E�B
		m_forwardRenderModel.Init(initData);
		//ZPrepass�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		//InitModelOnZprepass(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis);
		//�E�V�E��E��E�h�E�E�E�}�E�b�E�v�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		InitModelOnShadowMap(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis, false);
		// �E�􉽊w�E�f�E�[�E�^�E��E��E��E��E��E��E��E��E�B
		InitGeometryDatas(1);
		// �E��E��E�C�E�g�E��E��E��E��E�[�E��E��E�h�E�ɒǉ��E�B
		// g_renderingEngine->AddModelToRaytracingWorld(m_forwardRenderModel);
		// m_addRaytracingWorldModel = &m_forwardRenderModel;
		// �E�e�E�����[�E��E��E�h�E�s�E��E��E��E�X�E�V�E��E��E��E�B
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
		// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E��E��E��E��E��E��E��E�B
		InitInstancingDraw(maxInstance);
		// �E�X�E�P�E��E��E�g�E��E��E��E��E��E��E��E��E��E��E�B
		InitSkeleton(filePath);
		// �E�A�E�j�E��E��E�[�E�V�E��E��E��E��E��E��E��E��E��E��E��E��E�B
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// �E�A�E�j�E��E��E�[�E�V�E��E��E��E��E�ςݒ��E�_�E�o�E�b�E�t�E�@�E�̌v�E�Z�E��E��E��E��E��E��E��E��E��E��E��E��E�B
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		// GBuffer�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		InitModelOnRenderGBuffer(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		// ZPrepass�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		// �E�V�E��E��E�h�E�E�E�}�E�b�E�v�E�`�E��E�p�E�́E���E�f�E��E��E��E��E��E��E��E��E��E��E�B
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// �E�􉽊w�E�f�E�[�E�^�E��E��E��E��E��E��E��E��E�B
		InitGeometryDatas(maxInstance);
		// �E�e�E�����[�E��E��E�h�E�s�E��E��E��E�X�E�V�E��E��E��E�B
		UpdateWorldMatrixInModes();
		
		if (m_isRaytracingWorld) {
			// �E��E��E�C�E�g�E��E��E��E��E�[�E��E��E�h�E�ɒǉ��E�B
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
			// �E��E��E��E��E�_�E��E��E��E��E�O�E�G�E��E��E�W�E��E��E�ɓo�E�^�E�B
			g_renderingEngine->RegisterGeometryData(&geomData);
			instanceId++;
		}
	}
	void ModelRender::InitSkeleton(const char* filePath)
	{
		//�E�X�E�P�E��E��E�g�E��E��E�̃f�E�[�E�^�E��E�ǂݍ��E�݁B
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
			// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�̂ŁA
			// �E��E��E��E�p�E�̃f�E�[�E�^�E��E��E�\�E�z�E��E��E��E�B
			// �E��E��E�[�E��E��E�h�E�s�E��E�̔z�E��E�́E���E��E��E��E��E��E��E�m�E�ۂ��E��E�B
			m_worldMatrixArray = std::make_unique<Matrix[]>(m_maxInstance);
			// �E��E��E�[�E��E��E�h�E�s�E��E��E�GPU�E�ɓ]�E��E��E��E��E�邽�E�߂̃X�E�g�E��E��E�N�E�`�E��E��E�[�E�h�E�o�E�b�E�t�E�@�E��E��E�m�E�ہB
			m_worldMatrixArraySB.Init(
				sizeof(Matrix),
				m_maxInstance,
				nullptr
			);
			m_isEnableInstancingDraw = true;
			// �E�C�E��E��E�X�E�^�E��E��E�X�E�ԍ��E��E��E�烏�[�E��E��E�h�E�s�E��E�̔z�E��E�̃C�E��E��E�f�E�b�E�N�E�X�E�ɕϊ��E��E��E��E�e�E�[�E�u�E��E��E��E��E��E��E��E��E��E��E��E��E��E�B
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

		// �E��E��E�_�E�V�E�F�E�[�E�_�E�[�E�̃G�E��E��E�g�E��E��E�[�E�|�E�C�E��E��E�g�E��E��E�Z�E�b�E�g�E�A�E�b�E�v�E�B
		SetupVertexShaderEntryPointFunc(modelInitData);
		if (m_vsEntryOverride) {
			modelInitData.m_vsEntryPointFunc     = m_vsEntryOverride;
			modelInitData.m_vsSkinEntryPointFunc = m_vsEntryOverride;
		}
		// �E��E��E�_�E�̎��E�O�E�v�E�Z�E��E��E��E��E��E��E�g�E��E��E�B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;
		if (m_animationClips != nullptr) {
			//�E�X�E�P�E��E��E�g�E��E��E��E��E�w�E�肷�E��E�B
			modelInitData.m_skeleton = &m_skeleton;		}

		if (isShadowReciever) {
			modelInitData.m_psEntryPointFunc = "PSMainShadowReciever";
		}
		//�E��E��E�f�E��E��E�̏��E��E��E��E��E��E�w�E�肷�E��E�B
		modelInitData.m_modelUpAxis = enModelUpAxis;

		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_SNORM;
		modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (m_isEnableInstancingDraw) {
			// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�ꍁE��́A�E�g�E��E�SRV�E�ɃC�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E�ݒ肷�E��E�B
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

		// �E��E��E�_�E�V�E�F�E�[�E�_�E�[�E�̃G�E��E��E�g�E��E��E�[�E�|�E�C�E��E��E�g�E��E��E�Z�E�b�E�g�E�A�E�b�E�v�E�B
		SetupVertexShaderEntryPointFunc(modelInitData);
		// �E��E��E�_�E�̎��E�O�E�v�E�Z�E��E��E��E��E��E��E�g�E��E��E�B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//�E�X�E�P�E��E��E�g�E��E��E��E��E�w�E�肷�E��E�B
			modelInitData.m_skeleton = &m_skeleton;
		}

		if (g_renderingEngine->IsSoftShadow()) {
			modelInitData.m_psEntryPointFunc = "PSMainSoftShadow";
		}
		else {
			modelInitData.m_psEntryPointFunc = "PSMainHardShadow";
		}
		//�E��E��E�f�E��E��E�̏��E��E��E��E��E��E�w�E�肷�E��E�B
		modelInitData.m_modelUpAxis = enModelUpAxis;
		modelInitData.m_expandConstantBuffer = &g_renderingEngine->GetDeferredLightingCB();
		modelInitData.m_expandConstantBufferSize = sizeof(g_renderingEngine->GetDeferredLightingCB());
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		modelInitData.m_alphaBlendMode = AlphaBlendMode_Trans;

		int expandSRVNo = 0;
		if (m_isEnableInstancingDraw) {
			// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�ꍁE��́A�E�g�E��E�SRV�E�ɃC�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E�ݒ肷�E��E�B
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
		// �E��E��E�_�E�V�E�F�E�[�E�_�E�[�E�̃G�E��E��E�g�E��E��E�[�E�|�E�C�E��E��E�g�E��E��E�Z�E�b�E�g�E�A�E�b�E�v�E�B
		SetupVertexShaderEntryPointFunc(modelInitData);

		// �E��E��E�_�E�̎��E�O�E�v�E�Z�E��E��E��E��E��E��E�g�E��E��E�B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//�E�X�E�P�E��E��E�g�E��E��E��E��E�w�E�肷�E��E�B
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
			// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�ꍁE��́A�E�g�E��E�SRV�E�ɃC�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E�ݒ肷�E��E�B
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

		// �E��E��E�_�E�V�E�F�E�[�E�_�E�[�E�̃G�E��E��E�g�E��E��E�[�E�|�E�C�E��E��E�g�E��E��E�Z�E�b�E�g�E�A�E�b�E�v�E�B
		SetupVertexShaderEntryPointFunc(modelInitData);
		// �E��E��E�_�E�̎��E�O�E�v�E�Z�E��E��E��E��E��E��E�g�E��E��E�B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//�E�X�E�P�E��E��E�g�E��E��E��E��E�w�E�肷�E��E�B
			modelInitData.m_skeleton = &m_skeleton;
		}

		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (m_isEnableInstancingDraw) {
			// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�ꍁE��́A�E�g�E��E�SRV�E�ɃC�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�p�E�̃f�E�[�E�^�E��E�ݒ肷�E��E�B
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}

		m_zprepassModel.Init(modelInitData);
	}
	void ModelRender::UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
	{
		K2_ASSERT(instanceNo < m_maxInstance, "�E�C�E��E��E�X�E�^�E��E��E�X�E�ԍ��E��E��E�s�E��E��E�ł��E�B");
		if (!m_isEnableInstancingDraw) {
			return;
		}
		Matrix worldMatrix;
		if (m_translucentModel.IsInited()) {
			// �E��E��E��E��E��E��E��E��E�f�E��E��E��E�ZPrepass�E��E��E�f�E��E��E��E��E��E��E��E��E��E��E��E��E�Ă��E�Ȃ��E�̂ŁA�E��E��E��E��E��E��E��E�g�E��E��E�B
			worldMatrix = m_translucentModel.CalcWorldMatrix(pos, rot, scale);
		}
		else {
			worldMatrix = m_zprepassModel.CalcWorldMatrix(pos, rot, scale);
		} 
		// �E�C�E��E��E�X�E�^�E��E��E�X�E�ԍ��E��E��E��E�s�E��E�̃C�E��E��E�f�E�b�E�N�E�X�E��E��E�擾�E��E��E��E�B
		int matrixArrayIndex = m_instanceNoToWorldMatrixArrayIndexTable[instanceNo];
		// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E��E��E�s�E��E��E�B
		m_worldMatrixArray[matrixArrayIndex] = worldMatrix;
		if (m_numInstance == 0) {
			//�E�C�E��E��E�X�E�^�E��E��E�X�E��E��E��E�0�E�̏ꍇ�E�̂݃A�E�j�E��E��E�[�E�V�E��E��E��E��E�֌W�E�̍X�E�V�E��E��E�s�E��E��E�B
			// �E�X�E�P�E��E��E�g�E��E��E��E��E�X�E�V�E�B
			// �E�e�E�C�E��E��E�X�E�^�E��E��E�X�E�́E���E�[�E��E��E�h�E��E�Ԃւ̕ϊ��E�́A
			// �E�C�E��E��E�X�E�^�E��E��E�X�E��E��E�Ƃɍs�E��E��E�K�E�v�E��E��E��E��E��E�̂ŁA�E��E��E�_�E�V�E�F�E�[�E�_�E�[�E�ōs�E��E��E�B
			// �E�Ȃ̂ŁA�E�P�E�ʍs�E��E��E�n�E��E��E�āA�E��E��E�f�E��E��E��E�ԂŃ{�E�[�E��E��E�s�E��E��E��E�\�E�z�E��E��E��E�B
			m_skeleton.Update(g_matIdentity);
			//�E�A�E�j�E��E��E�[�E�V�E��E��E��E��E��E�i�E�߂�B
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

		//�E�A�E�j�E��E��E�[�E�V�E��E��E��E��E��E�i�E�߂�B
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);

	}
	void ModelRender::Draw(RenderContext& rc)
	{
		if (m_isEnableInstancingDraw) {
			// �E�C�E��E��E�X�E�^�E��E��E�V�E��E��E�O�E�`�E��E�̓r�E��E��E�[�E�t�E��E��E�X�E�^�E��E��E�J�E��E��E��E��E�O�E�͍s�E��E�Ȃ��E�B
			g_renderingEngine->AddRenderObject(this);
			m_worldMatrixArraySB.Update(m_worldMatrixArray.get());
			m_numInstance = 0;
		}
		else {
			// �E�ʏ�`�E��E�
			if (m_geometryDatas.empty() || m_geometryDatas[0].IsInViewFrustum()) {
				// �E�r�E��E��E�[�E�t�E��E��E�X�E�^�E��E��E�Ɋ܂܂�Ă��E��E�B
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
		// �E��E��E�_�E�̎��E�O�E�v�E�Z�E��E��E��E��E��E��E�f�E�B�E�X�E�p�E�b�E�`�E��E��E��E�B
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