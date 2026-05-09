#pragma once

#include "MyRenderer.h"
#include "geometry/AABB.h"
#include "geometry/geometryData.h"
#include "graphics/ComputeAnimationVertexBuffer.h"
#include "graphics/DitherCBData.h"

namespace nsK2Engine {
	class RenderingEngine;

	/// <summary>
	/// �X�L�����f�������_�[�B
	/// </summary>
	class ModelRender : public IRenderer
	{
	public:
		ModelRender();
		~ModelRender();
		/// <summary>
		/// �������B�ʏ�͂��̊֐��ŏ��������Ă��������B
		/// </summary>
		/// <param name="filePath">�t�@�C���p�X�B</param>
		/// <param name="animationClips">�A�j���[�V�����N���b�v�B</param>
		/// <param name="numAnimationClips">�A�j���[�V�����N���b�v�̐��B</param>
		/// <param name="enModelUpAxis">���f���̏�����B</param>
		/// <param name="isShadowReciever">�V���h�E���V�[�o�[�t���Otrue�Ȃ�e��������B</param>
		/// <param name="maxInstance">
		/// �C���X�^���X�̍ő吔�B���̈������P���傫������ƃC���X�^���V���O�`�悪�s���܂��B
		/// �C���X�^���V���O�`����s���ۂ͕`�悵�����C���X�^���X�̐��������AUpdateInstancingDraw()���Ăяo���K�v������܂��B
		/// �C���X�^���V���O�`��̏ڍׂ�Smaple_XX���Q�Ƃ��Ă��������B
		/// </param>
		/// <param name="isFrontCullingOnDrawShadowMap">
		/// �V���h�E�}�b�v��`�悷��ۂɃt�����g�J�����O���s���H
		/// �t�����g�J�����O���s���ƃV���h�E�A�N�l���y���ł��܂��B
		/// �������A�|���̂悤�ȕ��Ă��Ȃ����f�����t�����g�J�����O�ŕ`�悷��ƁA
		/// �V���h�E�}�b�v�ɕ`�悳��Ȃ��Ȃ��Ă��܂����߁A���̏ꍇ�͂��̃t���O��false�ɂ��Ă��������B
		/// ��)�n�ʂȂǁB
		/// </param>
		void Init(
			const char* filePath,
			AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnModelUpAxis enModelUpAxis = enModelUpAxisZ,
			bool isShadowReciever = true,
			int maxInstance = 1,
			bool isFrontCullingOnDrawShadowMap = false);
		/// <summary>
		/// �������`����s���I�u�W�F�N�g���������B
		/// </summary>
		/// <param name="filePath">�t�@�C���p�X�B</param>
		/// <param name="animationClips">�A�j���[�V�����N���b�v�B</param>
		/// <param name="numAnimationClips">�A�j���[�V�����N���b�v�̐��B</param>
		/// <param name="enModelUpAxis">���f���̏�����B</param>
		/// <param name="isShadowReciever">true�Ȃ�e��������B</param>
		/// <param name="maxInstance">
		/// �C���X�^���X�̍ő吔�B���̈������P���傫������ƃC���X�^���V���O�`�悪�s���܂��B
		/// �C���X�^���V���O�`����s���ۂ͕`�悵�����C���X�^���X�̐��������AUpdateInstancingDraw()���Ăяo���K�v������܂��B
		/// �C���X�^���V���O�`��̏ڍׂ�Smaple_XX���Q�Ƃ��Ă��������B
		/// </param>	
		/// <param name="isFrontCullingOnDrawShadowMap">
		/// �V���h�E�}�b�v��`�悷��ۂɃt�����g�J�����O���s���H
		/// �t�����g�J�����O���s���ƃV���h�E�A�N�l���y���ł��܂��B
		/// �������A�|���̂悤�ȕ��Ă��Ȃ����f�����t�����g�J�����O�ŕ`�悷��ƁA
		/// �V���h�E�}�b�v�ɕ`�悳��Ȃ��Ȃ��Ă��܂����߁A���̏ꍇ�͂��̃t���O��false�ɂ��Ă��������B
		/// ��)�n�ʂȂǁB
		/// </param>
		void IniTranslucent(
			const char* filePath,
			AnimationClip* animationClips = nullptr,
			int numAnimationClips = 0,
			EnModelUpAxis enModelUpAxis = enModelUpAxisZ,
			bool isShadowReciever = true,
			int maxInstance = 1,
			bool isFrontCullingOnDrawShadowMap = false);
		/// <summary>
		/// ����ȃV�F�[�f�B���O���s�������ꍇ�̏����������B
		/// </summary>
		/// <param name="initData">���f���f�[�^�B</param>
		void InitForwardRendering(ModelInitData& initData);
		/// <summary>
		/// �X�V�����B
		/// </summary>
		void Update();
		/// <summary>
		/// �C���X�^���V���O�f�[�^�̍X�V�B
		/// </summary>
		/// <param name="instanceNo">�C���X�^���X�ԍ�</param>
		/// <param name="pos">���W</param>
		/// <param name="rot">��]</param>
		/// <param name="scale">�g�嗦</param>
		void UpdateInstancingData(int instanceNo, const Vector3& pos, const Quaternion& rot, const Vector3& scale);
		/// <summary>
		/// �`�揈���B
		/// </summary>
		void Draw(RenderContext& rc);

		/// <summary>
		/// �A�j���[�V�����Đ��B
		/// </summary>
		/// <param name="animNo">�A�j���[�V�����N���b�v�̔ԍ��B</param>
		/// <param name="interpolateTime">�⊮����(�P�ʁF�b�B)</param>
		void PlayAnimation(int animNo, float interpolateTime = 0.0f)
		{
			m_animation.Play(animNo, interpolateTime);
		}

		/// <summary>
		/// �A�j���[�V�����̍Đ����H
		/// </summary>
		bool IsPlayingAnimation() const
		{
			return m_animation.IsPlaying();
		}

		/// <summary>
		/// ���f�����擾�B
		/// </summary>
		/// <returns>���f��</returns>
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
			// �����܂ŗ���̂͂��������B
			return m_zprepassModel;
		}

		/// <summary>
		/// ���W�A��]�A�g���S�Đݒ�B
		/// </summary>
		/// <remark>
		/// �C���X�^���V���O�`�悪�L���̏ꍇ�́A���̐ݒ�͖�������܂��B
		/// </remark>
		/// <param name="pos">���W�B</param>
		/// <param name="rotation">��]�B</param>
		/// <param name="scale">�g��B</param>
		void SetTRS(const Vector3& pos, const Quaternion& rotation, const Vector3& scale)
		{
			SetPosition(pos);
			SetRotation(rotation);
			SetScale(scale);
		}

		/// <summary>
		/// ���W��ݒ�B
		/// </summary>
		/// <remark>
		/// �C���X�^���V���O�`�悪�L���̏ꍇ�́A���̐ݒ�͖�������܂��B
		/// </remark>
		/// <param name="pos">���W�B</param>
		void SetPosition(const Vector3& pos)
		{
			m_position = pos;
		}
		/// <summary>
		/// ���W��ݒ�B
		/// </summary>
		/// <param name="x">x���W</param>
		/// <param name="y">y���W</param>
		/// <param name="z">z���W</param>
		void SetPosition(float x, float y, float z)
		{
			SetPosition({ x, y, z });
		}
		/// <summary>
		/// ��]��ݒ�B
		/// </summary>
		/// <remark>
		/// �C���X�^���V���O�`�悪�L���̏ꍇ�́A���̐ݒ�͖�������܂��B
		/// </remark>
		/// <param name="rotation">��]�B</param>
		void SetRotation(const Quaternion& rotation)
		{
			m_rotation = rotation;
		}

		/// <summary>
		/// �g�嗦��ݒ�B
		/// </summary>
		/// <remark>
		/// �C���X�^���V���O�`�悪�L���̏ꍇ�́A���̐ݒ�͖�������܂��B
		/// </remark>
		/// <param name="scale">�g�嗦�B</param>
		void SetScale(const Vector3& scale)
		{
			m_scale = scale;
		}
		void SetScale(float x, float y, float z)
		{
			SetScale({ x, y, z });
		}
		/// <summary>
		/// �V���h�E�L���X�^�[�̃t���O��ݒ肷��
		/// </summary>
		void SetShadowCasterFlag(bool flag)
		{
			m_isShadowCaster = flag;
		}
		/// <summary>
		/// �A�j���[�V�����Đ��̑��x��ݒ肷��B
		/// </summary>
		/// <param name="animationSpeed">���l�̕������{�ɂ���B</param>
		void SetAnimationSpeed(const float animationSpeed)
		{
			m_animationSpeed = animationSpeed;
		}
		/// <summary>
		/// �{�[���̖��O����{�[���ԍ��������B
		/// </summary>
		/// <param name="boneName">�{�[���̖��O</param>
		/// <returns>�{�[���ԍ��B������Ȃ������ꍇ��-1���Ԃ��Ă��܂��B</returns>
		int FindBoneID(const wchar_t* boneName) const
		{
			return m_skeleton.FindBoneID(boneName);
		}
		/// <summary>
		/// �{�[���ԍ�����{�[�����擾�B
		/// </summary>
		/// <param name="boneNo">�{�[���ԍ�</param>
		/// <returns>�{�[��</returns>
		Bone* GetBone(int boneNo) const
		{
			return m_skeleton.GetBone(boneNo);
		}
		void AddAnimationEvent(AnimationEventListener eventListener)
		{
			m_animation.AddAnimationEventListener(eventListener);
		}

		/// <summary>
		/// �V���h�E�L���X�^�[�H
		/// </summary>
		/// <returns></returns>
		bool IsShadowCaster() const
		{
			return m_isShadowCaster;
		}
		/// <summary>
		/// �C���X�^�������擾�B
		/// </summary>
		/// <returns></returns>
		int GetNumInstance() const
		{
			return m_numInstance;
		}
		/// <summary>
		/// �C���X�^���V���O�`����s���H
		/// </summary>
		/// <returns></returns>
		bool IsInstancingDraw() const
		{
			return m_isEnableInstancingDraw;
		}
		/// <summary>
		/// ���[���h�s����擾�B
		/// </summary>
		/// <param name="instanceId">
		/// �C���X�^���XID�B
		/// ���̈����̓C���X�^���V���O�`�悪�������̏ꍇ�͖�������܂��B
		/// </param>
		/// <returns></returns>
		const Matrix& GetWorldMatrix(int instanceId) const
		{
			if (IsInstancingDraw()) {
				return m_worldMatrixArray[instanceId];
			}
			return m_zprepassModel.GetWorldMatrix();
		}
		/// <summary>
		/// �C���X�^���X�������B
		/// </summary>
		/// <remark>
		/// �C���X�^���X�`��𗘗p���Ă���ۂɁA���̃C���X�^���X���V�[�����珜���������ꍇ�ɗ��p���Ă��������B
		/// </remark>
		/// <param name="instanceNo">�C���X�^���X�ԍ�</param>
		void RemoveInstance(int instanceNo);
		/// <summary>
		/// ���C�g�����[���h�ɓo�^���邩�̃t���O��ݒ肷��
		/// �p�ɂɐ������郂�f���i�e�Ȃǁj��Init����O�Ɉ���false�Ŏ��s���Ă��������B
		/// </summary>
		void SetRaytracingWorld(bool flag) {
			m_isRaytracingWorld = flag;
		}
		/// <summary>
		/// 透明度ディザ値を設定する(0.0=完全透明, 1.0=不透明)。
		/// GBufferパスでのみ有効。
		/// </summary>
		void SetDitherAlpha(float alpha)
		{
			m_localDitherCBData.ditherAlpha = alpha;
		}
		/// <summary>
		/// 透明度ディザ値を取得する。
		/// </summary>
		float GetDitherAlpha() const
		{
			return m_localDitherCBData.ditherAlpha;
		}
	private:
		/// <summary>
		/// �X�P���g���̏������B
		/// </summary>
		/// <param name="filePath">�t�@�C���p�X�B</param>
		void InitSkeleton(const char* filePath);
		/// <summary>
		/// �A�j���[�V�����̏������B
		/// </summary>
		/// <param name="animationClips">�A�j���[�V�����N���b�v�B</param>
		/// <param name="numAnimationClips">�A�j���[�V�����N���b�v�̐��B</param>
		/// <param name="enModelUpAxis">���f���̏�����B</param>
		void InitAnimation(AnimationClip* animationClips,
			int numAnimationClips,
			EnModelUpAxis enModelUpAxis);
		/// <summary>
		/// ���_�v�Z�p�X����Ăяo����鏈���B
		/// </summary>
		/// <param name="rc"></param>
		void OnComputeVertex(RenderContext& rc);
		/// <summary>
		/// �V���h�E�}�b�v�ւ̕`��p�X����Ă΂�鏈���B
		/// </summary>
		/// <param name="rc">�����_�����O�R���e�L�X�g</param>
		/// <param name="ligNo">���C�g�ԍ�</param>
		/// <param name="shadowMapNo">�V���h�E�}�b�v�ԍ�</param>
		/// <param name="lvpMatrix">���C�g�r���[�v���W�F�N�V�����s��</param>
		void OnRenderShadowMap(
			RenderContext& rc,
			int ligNo,
			int shadowMapNo,
			const Matrix& lvpMatrix) override;
		/// <summary>
		/// ZPrepass����Ă΂�鏈���B
		/// </summary>
		/// <param name="rc"></param>
		void OnZPrepass(RenderContext& rc) override;
		/// <summary>
		/// G-Buffer�`��p�X����Ă΂�鏈���B
		/// </summary>
		void OnRenderToGBuffer(RenderContext& rc) override;
		/// <summary>
		/// �t�H���[�h�����_�[�p�X����Ă΂�鏈���B
		/// </summary>
		void OnForwardRender(RenderContext& rc) override;
		/// <summary>
		/// �������I�u�W�F�N�g�`��p�X����Ă΂�鏈���B
		/// </summary>
		/// <param name="rc"></param>
		void OnTlanslucentRender(RenderContext& rc) override;
		/// <summary>
		/// �e�탂�f���̃��[���h�s����X�V����B
		/// </summary>
		void UpdateWorldMatrixInModes();
	private:
		/// <summary>
		/// �����_�����O�G���W���Ŕ��������C�x���g���󂯎�����Ƃ��ɌĂ΂�鏈���B
		/// </summary>
		/// <param name="enEvent"></param>
		void OnRecievedEventFromRenderingEngine(RenderingEngine::EnEvent enEvent);
		/// <summary>
		/// �V���h�E�}�b�v�`��p�̃��f�����������B
		/// </summary>
		/// <param name="renderingEngine">�����_�����O�G���W��</param>
		/// <param name="tkmFilePath">tkm�t�@�C���p�X</param>
		/// <param name="modelUpAxis">���f���̏㎲</param>
		void InitModelOnShadowMap(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis modelUpAxis,
			bool isFrontCullingOnDrawShadowMap
		);
		/// <summary>
		/// ZPrepass�`��p�̃��f�����������B
		/// </summary>
		/// <param name="renderingEngine"></param>
		/// <param name="tkmFilePath"></param>
		void InitModelOnZprepass(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis modelUpAxis
		);

		/// <summary>
		/// �C���X�^���V���O�`��p�̏��������������s�B
		/// </summary>
		/// <param name="maxInstance">�ő�C���X�^���X��</param>
		void InitInstancingDraw(int maxInstance);
		/// <summary>
		/// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������������B
		/// </summary>
		/// <param name="tkmFilePath">tkm�t�@�C���̃t�@�C���p�X</param>
		/// <param name="enModelUpAxis">���f���̏㎲</param>
		void InitComputeAnimatoinVertexBuffer(
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis);
		/// <summary>
		/// GBuffer�`��p�̃��f�����������B
		/// </summary>
		/// <param name="renderingEngine">�����_�����O�G���W��</param>
		/// <param name="tkmFilePath">tkm�t�@�C���p�X</param>
		void InitModelOnRenderGBuffer(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis,
			bool isShadowReciever);
		/// <summary>
		/// �������I�u�W�F�N�g�`��p�X�Ŏg�p����郂�f�����������B
		/// </summary>
		/// <param name="renderingEngine"></param>
		/// <param name="tkmFilePath"></param>
		/// <param name="enModelUpAxis"></param>
		/// <param name="isShadowReciever"></param>
		void InitModelOnTranslucent(
			RenderingEngine& renderingEngine,
			const char* tkmFilePath,
			EnModelUpAxis enModelUpAxis,
			bool isShadowReciever
		);
		/// <summary>
		/// �e�탂�f���̒��_�V�F�[�_�[�̃G���g���[�|�C���g��ݒ�B
		/// </summary>
		void SetupVertexShaderEntryPointFunc(ModelInitData& modelInitData);
		/// <summary>
		/// ���f��������o�E���f�B���O�{�����[�����������B
		/// </summary>
		/// <remark>
		/// AABB���\�z���܂��B
		/// </remark>
		void InitBoundingVolume();
		/// <summary>
		/// �􉽊w����������
		/// </summary>
		/// <param name="maxInstance">�C���X�^���X��</param>
		void InitGeometryDatas(int maxInstance);
	public:
		static const int NUM_SHADOW_LIGHT = 1;
	private:
		AnimationClip* m_animationClips = nullptr;			// �A�j���[�V�����N���b�v�B
		int							m_numAnimationClips = 0;			// �A�j���[�V�����N���b�v�̐��B
		Vector3 					m_position = Vector3::Zero;			// ���W�B
		Quaternion	 				m_rotation = Quaternion::Identity;	// ��]�B
		Vector3						m_scale = Vector3::One;				// �g�嗦�B
		EnModelUpAxis				m_enFbxUpAxis = enModelUpAxisZ;		// FBX�̏�����B
		Animation					m_animation;						// �A�j���[�V�����B
		ComputeAnimationVertexBuffer m_computeAnimationVertexBuffer;	// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�����B
		Model*						m_addRaytracingWorldModel = nullptr;// ���C�g�����[���h�ɓo�^�������f���B				
		Model						m_zprepassModel;					// ZPrepass�ŕ`�悳��郂�f��
		Model						m_forwardRenderModel;				// �t�H���[�h�����_�����O�̕`��p�X�ŕ`�悳��郂�f��
		Model						m_translucentModel;					// ���������f���B
		Model						m_renderToGBufferModel;				// RenderToGBuffer�ŕ`�悳��郂�f��
		Model						m_shadowModels[MAX_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];	// �V���h�E�}�b�v�ɕ`�悷�郂�f��
		ConstantBuffer				m_drawShadowMapCameraParamCB[MAX_DIRECTIONAL_LIGHT][NUM_SHADOW_MAP];		// �V���h�E�}�b�v�쐬���ɕK�v�ȃJ�����p�����[�^�p�̒萔�o�b�t�@�B
		bool						m_isUpdateAnimation = true;			// �A�j���[�V�������X�V����H
		Skeleton					m_skeleton;							// ���B
		bool						m_isShadowCaster = true;			// �V���h�E�L���X�^�[�t���O
		float						m_animationSpeed = 1.0f;
		int							m_numInstance = 0;					// �C���X�^���X�̐��B
		int							m_maxInstance = 1;					// �ő�C���X�^���X���B
		bool						m_isEnableInstancingDraw = false;	// �C���X�^���V���O�`�悪�L���H
		bool						m_isRaytracingWorld = true;			//���C�g�����[���h�ɓo�^����H
		std::unique_ptr<Matrix[]>	m_worldMatrixArray;					// ���[���h�s��̔z��B
		StructuredBuffer			m_worldMatrixArraySB;				// ���[���h�s��̔z��̃X�g���N�`���[�h�o�b�t�@�B
		std::vector< GemometryData > m_geometryDatas;					// �W�I���g�����B
		std::unique_ptr<int[]>		m_instanceNoToWorldMatrixArrayIndexTable;	// �C���X�^���X�ԍ����烏�[���h�s��̔z��̃C���f�b�N�X�ɕϊ�����e�[�u���B
		DitherCBData				m_localDitherCBData;						// per-object�̒萔�o�b�t�@(ditherAlpha�̓Z�b�^�[�ŕύX)

	};
}
