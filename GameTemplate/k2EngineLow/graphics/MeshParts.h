/// <summary>
/// ���b�V���p�[�c�N���X�B
/// </summary>

#pragma once

#include "tkFile/TkmFile.h"
#include "StructuredBuffer.h"

namespace nsK2EngineLow {

	class RenderContext;
	class Skeleton;
	class Material;
	class IShaderResource;
	class ComputeAnimationVertexBuffer;
	struct MaterialReInitData;


	const int MAX_MODEL_EXPAND_SRV = 32;	//�g��SRV�̍ő吔�B

	/// <summary>
	/// ���b�V��
	/// </summary>
	struct SMesh {
		VertexBuffer m_vertexBuffer;							// ���_�o�b�t�@�B
		std::vector< IndexBuffer* >		m_indexBufferArray;		// �C���f�b�N�X�o�b�t�@�B
		std::vector< Material* >		m_materials;			// �}�e���A���B
		std::vector<int>				skinFlags;				// �X�L���������Ă��邩�ǂ����̃t���O�B
	};

	/// <summary>
	/// ���b�V���p�[�c�B
	/// </summary>
	class MeshParts : public Noncopyable {
	public:
		/// <summary>
		/// �f�X�g���N�^�B
		/// </summary>
		~MeshParts();
		
		/// <summary>
		/// tkm�t�@�C�����珉����
		/// </summary>
		/// <param name="tkmFile">tkm�t�@�C���̃t�@�C���p�X</param>
		/// <param name="fxFilePath">�V�F�[�_�[�t�@�C���̃t�@�C���p�X</param>
		/// <param name="vsEntryPointFunc">�X�L���Ȃ����_�V�F�[�_�[�̃G���g���[�|�C���g��</param>
		/// <param name="vsSkinEntryPointFunc">�X�L�����蒸�_�V�F�[�_�[�̃G���g���[�|�C���g��</param>
		/// <param name="psEntryPointFunc">�s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g��</param>
		/// <param name="expandData">�g���萔�o�b�t�@�Bb1�Ƀo�C���h����܂��B</param>
		/// <param name="expandDataSize">�g���萔�o�b�t�@�̃T�C�Y</param>
		/// <param name="expandShaderResourceView">�g���V�F�[�_�[���\�[�X�r���[�̔z��</param>
		/// <param name="colorBufferFormat">�����_�����O�^�[�Q�b�g�̃J���[�o�b�t�@�t�H�[�}�b�g�̔z��B</param>
		/// <param name="alphaBlendMode">�A���t�@�u�����f�B���O���[�h</param>
		/// <param name="isDepthWrite">�[�x�o�b�t�@�ɏ������ނ��ǂ����̃t���O</param>
		/// <param name="isDepthTest">�[�x�e�X�g���s�������ۂ̃t���O�B</param>
		/// <param name="cullMode">�J�����O���[�h</param>
		/// <param name="computedAnimationVertexBuffer">�A�j���[�V�����ςݒ��_�o�b�t�@���v�Z���鏈���B</param>
		void InitFromTkmFile(
			const TkmFile& tkmFile,
			const char* fxFilePath,
			const char* vsEntryPointFunc,
			const char* vsSkinEntryPointFunc,
			const char* psEntryPointFunc,
			void* expandData,
			int expandDataSize,
			void* expandData2,
			int expandDataSize2,
			const std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV>& expandShaderResourceView,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode,
			ComputeAnimationVertexBuffer* computedAnimationVertexBuffer
		);
		/// <summary>
		/// �`��B
		/// </summary>
		/// <param name="rc">�����_�����O�R���e�L�X�g</param>
		/// <param name="mWorld">���[���h�s��</param>
		/// <param name="mView">�r���[�s��</param>
		/// <param name="mProj">�v���W�F�N�V�����s��</param>
		/// <param name="numInstance">�C���X�^���X�̐�</param>
		void Draw(
			RenderContext& rc,
			const Matrix& mWorld,
			const Matrix& mView,
			const Matrix& mProj,
			int numInstance);
		/// <summary>
		/// �X�P���g�����֘A�t����B
		/// </summary>
		/// <param name="skeleton">�X�P���g��</param>
		void BindSkeleton(Skeleton& skeleton);
		/// <summary>
		/// �A�j���[�V�����ςݒ��_�o�b�t�@�̌v�Z�������s���Ă���H
		/// </summary>
		/// <returns></returns>
		bool IsComputedAnimationVertexBuffer() const
		{
			return m_computedAnimationVertexBuffer != nullptr;
		}
		/// <summary>
		/// �A�j���[�V�����v�Z�ςݒ��_�o�b�t�@���擾�B
		/// </summary>
		/// <param name="meshNo">���b�V���̔ԍ�</param>
		/// <returns>���_�o�b�t�@</returns>
		const VertexBuffer& GetAnimatedVertexBuffer(int meshNo) const;
		VertexBuffer& GetAnimatedVertexBuffer(int meshNo);
		/// <summary>
		/// ���O�v�Z�ς݃C���f�b�N�X�o�b�t�@���擾
		/// </summary>
		/// <param name="meshNo">���b�V���ԍ�</param>
		/// <param name="matNo">�}�e���A���ԍ�</param>
		/// <returns></returns>
		const IndexBuffer& GetAnimatedIndexBuffer(int meshNo, int matNo) const;
		IndexBuffer& GetAnimatedIndexBuffer(int meshNo, int matNo);
		/// <summary>
		/// ���b�V���ɑ΂��Ė₢���킹���s���B
		/// </summary>
		/// <param name="queryFunc">�N�G���֐�</param>
		void QueryMeshs(std::function<void(const SMesh& mesh)> queryFunc)
		{
			for (const auto& mesh : m_meshs) {
				queryFunc(*mesh);
			}
		}
		void QueryMeshAndDescriptorHeap(std::function<void(const SMesh& mesh, const DescriptorHeap& ds)> queryFunc)
		{
			for (int i = 0; i < m_meshs.size(); i++) {
				queryFunc(*m_meshs[i], m_descriptorHeap);
			}
		}
		/// <summary>
		/// �f�B�X�N���v�^�q�[�v���쐬�B
		/// </summary>
		void CreateDescriptorHeaps();
		/// <summary>
		/// �}�e���A�����ď������B
		/// </summary>
		void ReInitMaterials(const MaterialReInitData& reInitData);
		/// <summary>
		/// ���b�V�����擾�B
		/// </summary>
		/// <param name="meshNo">���b�V���ԍ�</param>
		/// <returns>���b�V��</returns>
		const SMesh& GetMesh(int meshNo) const
		{
			return *m_meshs.at(meshNo);
		}
		SMesh& GetMesh(int meshNo)
		{
			return *m_meshs.at(meshNo);
		}
	private:
		/// <summary>
		/// tkm���b�V�����烁�b�V�����쐬�B
		/// </summary>
		/// <param name="mesh">���b�V��</param>
		/// <param name="meshNo">���b�V���ԍ�</param>
		/// <param name="fxFilePath">fx�t�@�C���̃t�@�C���p�X</param>
		/// <param name="vsEntryPointFunc">���_�V�F�[�_�[�̃G���g���[�|�C���g�̊֐���</param>
		/// <param name="vsSkinEntryPointFunc">�X�L������}�e���A���p�̒��_�V�F�[�_�[�̃G���g���[�|�C���g�̊֐���</param>
		/// <param name="psEntryPointFunc">�s�N�Z���V�F�[�_�[�̃G���g���[�|�C���g�̊֐���</param>
		/// <param name="colorBufferFormat">���̃��f���������_�����O����J���[�o�b�t�@�̃t�H�[�}�b�g�̔z��</param>
		/// <param name="alphaBlendMode">�A���t�@�u�����f�B���O���[�h</param>
		/// <param name="isDepthWrite">�[�x�l��[�x�o�b�t�@�ɏ������ނ��̃t���O</param>
		/// <param name="isDepthTest">�[�x�e�X�g���s�����ǂ����̃t���O</param>
		/// <param name="cullMode">�J�����O���[�h</param>
		/// <param name="computedAnimationVertexBuffer">�A�j���[�V�����ςݒ��_�o�b�t�@���v�Z���鏈���B</param>
		void CreateMeshFromTkmMesh(
			const TkmFile::SMesh& mesh,
			int meshNo,
			int& materialNum,
			const char* fxFilePath,
			const char* vsEntryPointFunc,
			const char* vsSkinEntryPointFunc,
			const char* psEntryPointFunc,
			const std::array<DXGI_FORMAT, MAX_RENDERING_TARGET>& colorBufferFormat,
			AlphaBlendMode alphaBlendMode,
			bool isDepthWrite,
			bool isDepthTest,
			D3D12_CULL_MODE cullMode,
			ComputeAnimationVertexBuffer* computedAnimationVertexBuffer
		);


	private:
		//�g��SRV���ݒ肳��郌�W�X�^�̊J�n�ԍ��B
		const int EXPAND_SRV_REG__START_NO = 10;
		//�P�̃}�e���A���Ŏg�p�����SRV�̐��B
		const int NUM_SRV_ONE_MATERIAL = EXPAND_SRV_REG__START_NO + MAX_MODEL_EXPAND_SRV;
		//�P�̃}�e���A���Ŏg�p�����CBV�̐��B
		const int NUM_CBV_ONE_MATERIAL = 3;
		/// <summary>
		/// �萔�o�b�t�@�B
		/// </summary>
		/// <remarks>
		/// ���̍\���̂�ύX������ASimpleModel.fx��CB���ύX����悤�ɁB
		/// </remarks>
		struct SConstantBuffer {
			Matrix mWorld;		//���[���h�s��B
			Matrix mView;		//�r���[�s��B
			Matrix mProj;		//�v���W�F�N�V�����s��B
		};
		ConstantBuffer m_commonConstantBuffer;					// ���b�V�����ʂ̒萔�o�b�t�@�B
		ConstantBuffer m_expandConstantBuffer;					// ���[�U�[�g���p�̒萔�o�b�t�@ (b1)
		ConstantBuffer m_expandConstantBuffer2;					// ���[�U�[�g���p�̒萔�o�b�t�@2 (b2) �X�v���b�g�p
		std::array<IShaderResource*, MAX_MODEL_EXPAND_SRV> m_expandShaderResourceView = { nullptr };	//���[�U�[�g���V�F�[�_�[���\�[�X�r���[�B
		StructuredBuffer m_boneMatricesStructureBuffer;			// �{�[���s��̍\�����o�b�t�@�B
		std::vector< SMesh* > m_meshs;							// ���b�V���B
		//std::vector< DescriptorHeap > m_descriptorHeap;		// �f�B�X�N���v�^�q�[�v�B
		DescriptorHeap m_descriptorHeap;						// �f�B�X�N���v�^�q�[�v�B
		Skeleton* m_skeleton = nullptr;							// �X�P���g���B
		void* m_expandData = nullptr;							// ���[�U�[�g���f�[�^�B (b1)
		void* m_expandData2 = nullptr;							// ���[�U�[�g���f�[�^2 (b2)
		ComputeAnimationVertexBuffer* m_computedAnimationVertexBuffer = nullptr;	// �A�j���[�V�����ςݒ��_�o�b�t�@���v�Z���鏈���B
	};
}