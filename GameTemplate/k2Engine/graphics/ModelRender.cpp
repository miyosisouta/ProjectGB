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
				// �����_�����O�G���W������o�^����
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
			// IBL�e�N�X�`�����X�V���ꂽ�̂ŁAPBR�V�F�[�_�[�𗘗p���Ă���
			// �t�H���[�h�����_�����O�̏ꍇ�́A�f�B�X�N���v�^�q�[�v���ď���������B
			// (IBL�e�N�X�`�����g���Ă���̂ŁB)
			MaterialReInitData matReInitData;
			if(m_isEnableInstancingDraw) {
				// �C���X�^���V���O�`����s���ꍇ�́A�g��SRV�ɃC���X�^���V���O�`��p�̃f�[�^��ݒ肷��B
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
			// �A�j���[�V��������B
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
		//�C���X�^���V���O�`��p�̃f�[�^���������B
		InitInstancingDraw(maxInstance);
		//�X�P���g�����������B
		InitSkeleton(filePath);
		//�A�j���[�V�������������B
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������������B
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		//�������I�u�W�F�N�g�`��p�X�Ŏg�p����郂�f�����������B
		InitModelOnTranslucent(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		//ZPrepass�`��p�̃��f�����������B
		// InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		//�V���h�E�}�b�v�`��p�̃��f�����������B
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// �􉽊w�f�[�^���������B
		InitGeometryDatas(maxInstance);
		if (m_isRaytracingWorld) {
			// ���C�g�����[���h�ɒǉ��B
			g_renderingEngine->AddModelToRaytracingWorld(m_translucentModel);
			m_addRaytracingWorldModel = &m_translucentModel;
		}
		
		// �e�탏�[���h�s����X�V����B
		UpdateWorldMatrixInModes();

	}

	void ModelRender::InitForwardRendering(ModelInitData& initData)
	{
		//�C���X�^���V���O�`��p�̃f�[�^���������B
		InitInstancingDraw(1);
		InitSkeleton(initData.m_tkmFilePath);

		// todo �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������������B
		InitComputeAnimatoinVertexBuffer(initData.m_tkmFilePath, initData.m_modelUpAxis);

		initData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		//�쐬�����������f�[�^�����ƂɃ��f��������������B
		m_forwardRenderModel.Init(initData);
		//ZPrepass�`��p�̃��f�����������B
		//InitModelOnZprepass(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis);
		//�V���h�E�}�b�v�`��p�̃��f�����������B
		InitModelOnShadowMap(*g_renderingEngine, initData.m_tkmFilePath, initData.m_modelUpAxis, false);
		// �􉽊w�f�[�^���������B
		InitGeometryDatas(1);
		// ���C�g�����[���h�ɒǉ��B
		// g_renderingEngine->AddModelToRaytracingWorld(m_forwardRenderModel);
		// m_addRaytracingWorldModel = &m_forwardRenderModel;
		// �e�탏�[���h�s����X�V����B
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
		// �C���X�^���V���O�`��p�̃f�[�^���������B
		InitInstancingDraw(maxInstance);
		// �X�P���g�����������B
		InitSkeleton(filePath);
		// �A�j���[�V�������������B
		InitAnimation(animationClips, numAnimationClips, enModelUpAxis);
		// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������������B
		InitComputeAnimatoinVertexBuffer(filePath, enModelUpAxis);
		// GBuffer�`��p�̃��f�����������B
		InitModelOnRenderGBuffer(*g_renderingEngine, filePath, enModelUpAxis, isShadowReciever);
		// ZPrepass�`��p�̃��f�����������B
		InitModelOnZprepass(*g_renderingEngine, filePath, enModelUpAxis);
		// �V���h�E�}�b�v�`��p�̃��f�����������B
		InitModelOnShadowMap(*g_renderingEngine, filePath, enModelUpAxis, isFrontCullingOnDrawShadowMap);
		// �􉽊w�f�[�^���������B
		InitGeometryDatas(maxInstance);
		// �e�탏�[���h�s����X�V����B
		UpdateWorldMatrixInModes();
		
		if (m_isRaytracingWorld) {
			// ���C�g�����[���h�ɒǉ��B
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
			// �����_�����O�G���W���ɓo�^�B
			g_renderingEngine->RegisterGeometryData(&geomData);
			instanceId++;
		}
	}
	void ModelRender::InitSkeleton(const char* filePath)
	{
		//�X�P���g���̃f�[�^��ǂݍ��݁B
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
			// �C���X�^���V���O�`����s���̂ŁA
			// ����p�̃f�[�^���\�z����B
			// ���[���h�s��̔z��̃��������m�ۂ���B
			m_worldMatrixArray = std::make_unique<Matrix[]>(m_maxInstance);
			// ���[���h�s���GPU�ɓ]�����邽�߂̃X�g���N�`���[�h�o�b�t�@���m�ہB
			m_worldMatrixArraySB.Init(
				sizeof(Matrix),
				m_maxInstance,
				nullptr
			);
			m_isEnableInstancingDraw = true;
			// �C���X�^���X�ԍ����烏�[���h�s��̔z��̃C���f�b�N�X�ɕϊ�����e�[�u��������������B
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
		modelInitData.m_fxFilePath = "Assets/shader/preProcess/RenderToGBufferFor3DModel.fx";

		m_localDitherCBData = g_ditherCBData;
		modelInitData.m_expandConstantBuffer = &m_localDitherCBData;
		modelInitData.m_expandConstantBufferSize = sizeof(DitherCBData);

		// ���_�V�F�[�_�[�̃G���g���[�|�C���g���Z�b�g�A�b�v�B
		SetupVertexShaderEntryPointFunc(modelInitData);
		// ���_�̎��O�v�Z�������g���B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;
		if (m_animationClips != nullptr) {
			//�X�P���g�����w�肷��B
			modelInitData.m_skeleton = &m_skeleton;		}

		if (isShadowReciever) {
			modelInitData.m_psEntryPointFunc = "PSMainShadowReciever";
		}
		//���f���̏�������w�肷��B
		modelInitData.m_modelUpAxis = enModelUpAxis;

		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		modelInitData.m_colorBufferFormat[1] = DXGI_FORMAT_R8G8B8A8_SNORM;
		modelInitData.m_colorBufferFormat[2] = DXGI_FORMAT_R8G8B8A8_UNORM;

		if (m_isEnableInstancingDraw) {
			// �C���X�^���V���O�`����s���ꍇ�́A�g��SRV�ɃC���X�^���V���O�`��p�̃f�[�^��ݒ肷��B
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
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

		// ���_�V�F�[�_�[�̃G���g���[�|�C���g���Z�b�g�A�b�v�B
		SetupVertexShaderEntryPointFunc(modelInitData);
		// ���_�̎��O�v�Z�������g���B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//�X�P���g�����w�肷��B
			modelInitData.m_skeleton = &m_skeleton;
		}

		if (g_renderingEngine->IsSoftShadow()) {
			modelInitData.m_psEntryPointFunc = "PSMainSoftShadow";
		}
		else {
			modelInitData.m_psEntryPointFunc = "PSMainHardShadow";
		}
		//���f���̏�������w�肷��B
		modelInitData.m_modelUpAxis = enModelUpAxis;
		modelInitData.m_expandConstantBuffer = &g_renderingEngine->GetDeferredLightingCB();
		modelInitData.m_expandConstantBufferSize = sizeof(g_renderingEngine->GetDeferredLightingCB());
		modelInitData.m_tkmFilePath = tkmFilePath;
		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R16G16B16A16_FLOAT;
		modelInitData.m_alphaBlendMode = AlphaBlendMode_Trans;

		int expandSRVNo = 0;
		if (m_isEnableInstancingDraw) {
			// �C���X�^���V���O�`����s���ꍇ�́A�g��SRV�ɃC���X�^���V���O�`��p�̃f�[�^��ݒ肷��B
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
		// ���_�V�F�[�_�[�̃G���g���[�|�C���g���Z�b�g�A�b�v�B
		SetupVertexShaderEntryPointFunc(modelInitData);

		// ���_�̎��O�v�Z�������g���B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//�X�P���g�����w�肷��B
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
			// �C���X�^���V���O�`����s���ꍇ�́A�g��SRV�ɃC���X�^���V���O�`��p�̃f�[�^��ݒ肷��B
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

		// ���_�V�F�[�_�[�̃G���g���[�|�C���g���Z�b�g�A�b�v�B
		SetupVertexShaderEntryPointFunc(modelInitData);
		// ���_�̎��O�v�Z�������g���B
		modelInitData.m_computedAnimationVertexBuffer = &m_computeAnimationVertexBuffer;

		if (m_animationClips != nullptr) {
			//�X�P���g�����w�肷��B
			modelInitData.m_skeleton = &m_skeleton;
		}

		modelInitData.m_colorBufferFormat[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;
		if (m_isEnableInstancingDraw) {
			// �C���X�^���V���O�`����s���ꍇ�́A�g��SRV�ɃC���X�^���V���O�`��p�̃f�[�^��ݒ肷��B
			modelInitData.m_expandShaderResoruceView[0] = &m_worldMatrixArraySB;
		}

		m_zprepassModel.Init(modelInitData);
	}
	void ModelRender::UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale)
	{
		K2_ASSERT(instanceNo < m_maxInstance, "�C���X�^���X�ԍ����s���ł��B");
		if (!m_isEnableInstancingDraw) {
			return;
		}
		Matrix worldMatrix;
		if (m_translucentModel.IsInited()) {
			// ���������f����ZPrepass���f�������������Ă��Ȃ��̂ŁA��������g���B
			worldMatrix = m_translucentModel.CalcWorldMatrix(pos, rot, scale);
		}
		else {
			worldMatrix = m_zprepassModel.CalcWorldMatrix(pos, rot, scale);
		} 
		// �C���X�^���X�ԍ�����s��̃C���f�b�N�X���擾����B
		int matrixArrayIndex = m_instanceNoToWorldMatrixArrayIndexTable[instanceNo];
		// �C���X�^���V���O�`����s���B
		m_worldMatrixArray[matrixArrayIndex] = worldMatrix;
		if (m_numInstance == 0) {
			//�C���X�^���X����0�̏ꍇ�̂݃A�j���[�V�����֌W�̍X�V���s���B
			// �X�P���g�����X�V�B
			// �e�C���X�^���X�̃��[���h��Ԃւ̕ϊ��́A
			// �C���X�^���X���Ƃɍs���K�v������̂ŁA���_�V�F�[�_�[�ōs���B
			// �Ȃ̂ŁA�P�ʍs���n���āA���f����ԂŃ{�[���s����\�z����B
			m_skeleton.Update(g_matIdentity);
			//�A�j���[�V������i�߂�B
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

		//�A�j���[�V������i�߂�B
		m_animation.Progress(g_gameTime->GetFrameDeltaTime() * m_animationSpeed);

	}
	void ModelRender::Draw(RenderContext& rc)
	{
		if (m_isEnableInstancingDraw) {
			// �C���X�^���V���O�`��̓r���[�t���X�^���J�����O�͍s��Ȃ��B
			g_renderingEngine->AddRenderObject(this);
			m_worldMatrixArraySB.Update(m_worldMatrixArray.get());
			m_numInstance = 0;
		}
		else {
			// �ʏ�`��
			if (m_geometryDatas.empty() || m_geometryDatas[0].IsInViewFrustum()) {
				// �r���[�t���X�^���Ɋ܂܂�Ă���B
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
		// ���_�̎��O�v�Z�������f�B�X�p�b�`����B
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
			// alphaだけ保持してグローバルの値（カメラ座標など）を毎フレームコピー
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