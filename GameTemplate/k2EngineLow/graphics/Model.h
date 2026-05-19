#pragma once

#include "tkFile/TkmFile.h"
#include "MeshParts.h"
#include "Skeleton.h"

namespace nsK2EngineLow {
	class IShaderResource;
	class ComputeAnimationVertexBuffer;


	/// <summary>
	/// ���f���̏������f�[�^
	/// </summary>
	struct ModelInitData {
		const char* m_tkmFilePath = nullptr;							// tkm�t�@�C���p�X�B
		const char* m_vsEntryPointFunc = "VSMain";						// ���_�V�F�[�_�[�̃G���g���[�|�C���g�B
		const char* m_vsSkinEntryPointFunc = "VSMain";					// �X�L������}�e���A���p�̒��_�V�F�[�_�[�̃G���g���[�|�C���g�B
		const char* m_psEntryPointFunc = "PSMain";						// �s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g�B
		const char* m_fxFilePath = nullptr;								// .fx�t�@�C���̃t�@�C���p�X�B
		void* m_expandConstantBuffer = nullptr;							// ���[�U�[�g���̒萔�o�b�t�@�B (b1)
		int m_expandConstantBufferSize = 0;								// ���[�U�[�g���̒萔�o�b�t�@�̃T�C�Y�B
		void* m_expandConstantBuffer2 = nullptr;						// ���[�U�[�g���̒萔�o�b�t�@2 (b2) �X�v���b�g�p
		int m_expandConstantBufferSize2 = 0;							// ���[�U�[�g���̒萔�o�b�t�@2�̃T�C�Y
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResoruceView = { nullptr };	// ���[�U�[�g���̃V�F�[�_�[���\�[�X�B
																										// t10���W�X�^���珇�ԂɊ��蓖�Ă��܂��B
		Skeleton* m_skeleton = nullptr;									// �X�P���g���B
		EnModelUpAxis m_modelUpAxis = enModelUpAxisZ;					// ���f���̏�����B
		AlphaBlendMode m_alphaBlendMode = AlphaBlendMode_None;			// �A���t�@�u�����f�B���O���[�h�B
		bool m_isDepthWrite = true;										// �[�x�o�b�t�@�ɏ������ށH
		bool m_isDepthTest = true;										// �[�x�e�X�g���s���H
		D3D12_CULL_MODE m_cullMode = D3D12_CULL_MODE_BACK;				// �J�����O���[�h�B
		ComputeAnimationVertexBuffer* m_computedAnimationVertexBuffer = nullptr;	// �A�j���[�V�����ςݒ��_�o�b�t�@���v�Z���鏈���B

		std::array<DXGI_FORMAT, MAX_RENDERING_TARGET> m_colorBufferFormat = {
			DXGI_FORMAT_R8G8B8A8_UNORM,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
			DXGI_FORMAT_UNKNOWN,
		};	//�����_�����O����J���[�o�b�t�@�̃t�H�[�}�b�g�B
		
	};
	/// <summary>
	/// �}�e���A�����ď��������邽�߂̃f�[�^�B
	/// </summary>
	struct MaterialReInitData {
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResoruceView = { nullptr };
	};
	/// <summary>
	/// ���f���N���X�B
	/// </summary>
	class Model : public Noncopyable {

	public:

		/// <summary>
		/// tkm�t�@�C�����珉�����B
		/// </summary>
		/// <param name="initData">�������f�[�^</param>
		void Init(const ModelInitData& initData);
		/// <summary>
		/// ���[���h�s��̍X�V�B
		/// </summary>
		/// <param name="pos">���W</param>
		/// <param name="rot">��]</param>
		/// <param name="scale">�g�嗦</param>
		void UpdateWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale)
		{
			m_worldMatrix = CalcWorldMatrix(pos, rot, scale);
		}

		/// <summary>
		/// ���[���h�s����v�Z
		/// </summary>
		/// <remark>
		/// Model�N���X�̐ݒ�Ɋ�Â������[���h�s��̌v�Z���s���܂��B
		/// �v�Z���ꂽ���[���h�s�񂪖߂�l�Ƃ��ĕԂ���܂��B
		/// �{�֐���UpdateWorldMatrix����Ă΂�Ă��܂��B
		/// �{�֐��̓��[���h�s����v�Z���āA�߂������ł��B
		/// Model::m_worldMatrix���X�V�����킯�ł͂Ȃ��̂ŁA���ӂ��Ă��������B
		/// �{�N���X�̐ݒ�Ɋ�Â��Čv�Z���ꂽ���[���h�s�񂪕K�v�ȏꍇ�Ɏg�p���Ă�������
		/// </remark>
		/// <param name="pos">���W</param>
		/// <param name="rot">��]</param>
		/// <param name="scale">�g�嗦</param>
		/// <returns>���[���h�s��</returns>
		Matrix CalcWorldMatrix(Vector3 pos, Quaternion rot, Vector3 scale);
		/// <summary>
		/// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������f�B�X�p�b�`�B
		/// </summary>
		/// <param name="rc">�����_�����O�R���e�L�X�g</param>
		void DispatchComputeAnimatedVertexBuffer(RenderContext& rc);
		/// <summary>
		/// �`��
		/// </summary>
		/// <param name="renderContext">�����_�����O�R���e�L�X�g</param>
		/// <param name="numInstance">�C���X�^���X�̐�</param>
		void Draw(
			RenderContext& renderContext,
			int numInstance = 1
		);
		/// <summary>
		/// �`��(�J�����w���)
		/// </summary>
		/// <param name="renderContext">�����_�����O�R���e�L�X�g</param>
		/// <param name="camera">�J����</param>
		/// <param name="numInstance">�C���X�^���X�̐�</param>
		void Draw(
			RenderContext& renderContext,
			Camera& camera,
			int numInstance = 1
		);
		/// <summary>
		/// �`��(�J�����s��w���)
		/// </summary>
		/// <param name="renderContext">�����_�����O�R���e�L�X�g</param>
		/// <param name="viewMatrix">�r���[�s��</param>
		/// <param name="projMatrix">�v���W�F�N�V�����s��</param>
		/// <param name="numInstance">�C���X�^���X�̐�</param>
		void Draw(
			RenderContext& renderContext,
			const Matrix& viewMatrix,
			const Matrix& projMatrix,
			int numInstance = 1
		);
		/// <summary>
		/// ���[���h�s����擾�B
		/// </summary>
		/// <returns></returns>
		const Matrix& GetWorldMatrix() const
		{
			return m_worldMatrix;
		}
		/// <summary>
		/// ���b�V���ɑ΂��Ė₢���킹���s���B
		/// </summary>
		/// <param name="queryFunc">�₢���킹�֐�</param>
		void QueryMeshs(std::function<void(const SMesh& mesh)> queryFunc)
		{
			m_meshParts.QueryMeshs(queryFunc);
		}
		void QueryMeshAndDescriptorHeap(std::function<void(const SMesh& mesh, const DescriptorHeap& ds)> queryFunc)
		{
			m_meshParts.QueryMeshAndDescriptorHeap(queryFunc);
		}
		/// <summary>
		/// ���b�V�����擾�B
		/// </summary>
		/// <param name="meshNo">���b�V���ԍ�</param>
		/// <returns>���b�V��</returns>
		const SMesh& GetMesh(int meshNo) const
		{
			return m_meshParts.GetMesh(meshNo);
		}
		SMesh& GetMesh(int meshNo) 
		{
			return m_meshParts.GetMesh(meshNo);
		}
		/// <summary>
		/// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������s���Ă���H
		/// </summary>
		/// <returns></returns>
		bool IsComputedAnimationVertexBuffer() const
		{
			return m_meshParts.IsComputedAnimationVertexBuffer();
		}
		/// <summary>
		/// �A�j���[�V�����v�Z�ςݒ��_�o�b�t�@���擾�B
		/// </summary>
		/// <param name="meshNo">���b�V���̔ԍ�</param>
		/// <returns>���_�o�b�t�@</returns>
		const VertexBuffer& GetAnimatedVertexBuffer(int meshNo) const
		{
			return m_meshParts.GetAnimatedVertexBuffer(meshNo);
		}
		VertexBuffer& GetAnimatedVertexBuffer(int meshNo)
		{
			return m_meshParts.GetAnimatedVertexBuffer(meshNo);
		}
		/// <summary>
		/// ���O�v�Z�ς݃C���f�b�N�X�o�b�t�@���擾
		/// </summary>
		/// <param name="meshNo">���b�V���ԍ�</param>
		/// <param name="matNo">�}�e���A���ԍ�</param>
		/// <returns></returns>
		const IndexBuffer& GetAnimatedIndexBuffer(int meshNo, int matNo) const
		{
			return m_meshParts.GetAnimatedIndexBuffer(meshNo, matNo);
		}
		IndexBuffer& GetAnimatedIndexBuffer(int meshNo, int matNo)
		{
			return m_meshParts.GetAnimatedIndexBuffer(meshNo, matNo);
		}
		/// <summary>
		/// �A���x�h�}�b�v��ύX�B
		/// </summary>
		/// <remarks>
		/// ���̊֐����Ăяo���ƃf�B�X�N���v�^�q�[�v�̍č\�z���s���邽�߁A�������ׂ�������܂��B
		/// ���t���[���Ăяo���K�v���Ȃ��ꍇ�͌Ăяo���Ȃ��悤�ɂ��Ă��������B
		/// </remarks>
		/// <param name="materialName">�ύX�������}�e���A���̖��O</param>
		/// <param name="albedoMap">�A���x�h�}�b�v</param>
		void ChangeAlbedoMap(const char* materialName, Texture& albedoMap);
		/// <summary>
		/// TKM�t�@�C�����擾�B
		/// </summary>
		/// <returns></returns>
		const TkmFile& GetTkmFile() const
		{
			return *m_tkmFile;
		}
		/// <summary>
		/// ����������Ă��邩����B
		/// </summary>
		/// <returns></returns>
		bool IsInited() const
		{
			return m_isInited;
		}
		/// <summary>
		/// �}�e���A�����ď������B
		/// </summary>
		/// <remark>
		/// ���f���ɓ\��t����e�N�X�`����ύX�������ꍇ�Ȃǂɗ��p���Ă��������B
		/// </remark>
		/// <param name="reInitData">�ď������f�[�^�B</param>
		void ReInitMaterials(MaterialReInitData& reInitData);
	private:
		bool m_isInited = false;							// ����������Ă���H
		Matrix m_worldMatrix;								// ���[���h�s��B
		TkmFile* m_tkmFile;									// tkm�t�@�C���B
		Skeleton m_skeleton;								// �X�P���g���B
		MeshParts m_meshParts;								// ���b�V���p�[�c�B
		EnModelUpAxis m_modelUpAxis = enModelUpAxisY;		// ���f���̏�����B
		Model* m_computedAnimationVertexBuffer = nullptr;	// �A�j���[�V�����㒸�_�o�b�t�@���v�Z�ς݂̃��f���B
	};
}
