#include "k2EngineLowPreCompile.h"
#include "MeshParts.h"
#include "Skeleton.h"
#include "Material.h"
#include "IndexBuffer.h"
#include "ComputeAnimationVertexBuffer.h"

namespace nsK2EngineLow {
	static TResourceBank<Material>& GetMaterialBank()
	{
		static TResourceBank<Material> materialBank;
		return materialBank;
	}
	MeshParts::~MeshParts()
	{
		for (auto& mesh : m_meshs) {
			//�C���f�b�N�X�o�b�t�@���폜�B
			for (auto& ib : mesh->m_indexBufferArray) {
				delete ib;
			}
			//���b�V�����폜�B
			delete mesh;
		}
	}
	void MeshParts::InitFromTkmFile(
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
	)
	{
		m_meshs.resize(tkmFile.GetNumMesh());
		int meshNo = 0;
		int materianNo = 0;
		tkmFile.QueryMeshParts([&](const TkmFile::SMesh& mesh) {
			//tkm�t�@�C���̃��b�V����񂩂烁�b�V�����쐬����B
			CreateMeshFromTkmMesh(
				mesh,
				meshNo,
				materianNo,
				fxFilePath,
				vsEntryPointFunc,
				vsSkinEntryPointFunc,
				psEntryPointFunc,
				colorBufferFormat,
				alphaBlendMode,
				isDepthWrite,
				isDepthTest,
				cullMode,
				computedAnimationVertexBuffer
			);
			meshNo++;
		});
		//���ʒ萔�o�b�t�@�̍쐬�B
		m_commonConstantBuffer.Init(sizeof(SConstantBuffer), nullptr);
		//���[�U�[�g���p�̒萔�o�b�t�@���쐬�B
		if (expandData) {
			m_expandConstantBuffer.Init(expandDataSize, nullptr);
			m_expandData = expandData;
		}
		if (expandData2) {
			m_expandConstantBuffer2.Init(expandDataSize2, nullptr);
			m_expandData2 = expandData2;
		}
		for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++) {
			m_expandShaderResourceView[i] = expandShaderResourceView[i];
		}
		//�f�B�X�N���v�^�q�[�v���쐬�B
		CreateDescriptorHeaps();
	}
	void MeshParts::ReInitMaterials(const MaterialReInitData& reInitData)
	{
		for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++) {
			m_expandShaderResourceView[i] = reInitData.m_expandShaderResoruceView[i];
		}
		//�f�B�X�N���v�^�q�[�v���쐬�B
		CreateDescriptorHeaps();
	}
	void MeshParts::CreateDescriptorHeaps()
	{
		// �K�v�ȃf�B�X�N���v�^�q�[�v�̑������v�Z����B
		int srvNo = 0;
		int cbNo = 0;
		for (auto& mesh : m_meshs) {
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				srvNo += NUM_SRV_ONE_MATERIAL;
				cbNo += NUM_CBV_ONE_MATERIAL;
			}
		}
		// �V�F�[�_�[���\�[�X�r���[�ƒ萔�o�b�t�@�̓o�^�ł���T�C�Y�����T�C�Y����B
		m_descriptorHeap.ResizeShaderResource(srvNo);
		m_descriptorHeap.ResizeConstantBuffer(cbNo);
		// UAV����Ȃ��B
		m_descriptorHeap.ResizeUnorderAccessResource(0);
		//�f�B�X�N���v�^�q�[�v���\�z���Ă����B
		srvNo = 0;
		cbNo = 0;
		for (auto& mesh : m_meshs) {
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {

				//�f�B�X�N���v�^�q�[�v�Ƀf�B�X�N���v�^��o�^���Ă����B
				m_descriptorHeap.RegistShaderResource(srvNo, mesh->m_materials[matNo]->GetAlbedoMap());		//�A���x�h�}�b�v�B
				m_descriptorHeap.RegistShaderResource(srvNo + 1, mesh->m_materials[matNo]->GetNormalMap());		//�@���}�b�v�B
				m_descriptorHeap.RegistShaderResource(srvNo + 2, mesh->m_materials[matNo]->GetSpecularMap());		//�X�y�L�����}�b�v�B
				m_descriptorHeap.RegistShaderResource(srvNo + 3, m_boneMatricesStructureBuffer);							//�{�[���̃X�g���N�`���[�h�o�b�t�@�B
				for (int i = 0; i < MAX_MODEL_EXPAND_SRV; i++) {
					if (m_expandShaderResourceView[i]) {
						m_descriptorHeap.RegistShaderResource(srvNo + EXPAND_SRV_REG__START_NO + i, *m_expandShaderResourceView[i]);
					}
				}
				srvNo += NUM_SRV_ONE_MATERIAL;
				m_descriptorHeap.RegistConstantBuffer(cbNo, m_commonConstantBuffer);
				if (m_expandConstantBuffer.IsValid()) {
					m_descriptorHeap.RegistConstantBuffer(cbNo + 1, m_expandConstantBuffer);
				}
				if (m_expandConstantBuffer2.IsValid()) {
					m_descriptorHeap.RegistConstantBuffer(cbNo + 2, m_expandConstantBuffer2);
				}
				cbNo += NUM_CBV_ONE_MATERIAL;
			}
		}
		m_descriptorHeap.Commit();
	}
	void MeshParts::CreateMeshFromTkmMesh(
		const TkmFile::SMesh& tkmMesh,
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
	) {
		//1. ���_�o�b�t�@���쐬�B
		int numVertex = (int)tkmMesh.vertexBuffer.size();
		int vertexStride = sizeof(TkmFile::SVertex);
		auto mesh = new SMesh;
		mesh->skinFlags.reserve(tkmMesh.materials.size());

		mesh->m_vertexBuffer.Init(vertexStride * numVertex, vertexStride);
		mesh->m_vertexBuffer.Copy((void*)&tkmMesh.vertexBuffer[0]);
		/*
		if (dispatchComputedAnimationVertexBuffer) {
			// �A�j���[�V�����ςݒ��_���L�����邽�߂̃o�b�t�@���쐬�B
			mesh->m_animatedVertexBuffer.Init(vertexStride * numVertex, vertexStride);
			mesh->m_animatedVertexBuffer.Copy((void*)&tkmMesh.vertexBuffer[0]);
			// �A�j���[�V�����ςݒ��_�o�b�t�@��RWStructuredBuffer���������B
			mesh->m_animatedVertexBufferRWSB.Init(mesh->m_animatedVertexBuffer, false);
		}
		*/
		m_computedAnimationVertexBuffer = computedAnimationVertexBuffer;
		
		auto SetSkinFlag = [&](int index) {
			if (tkmMesh.vertexBuffer[index].skinWeights.x > 0.0f) {
				//�X�L��������B
				mesh->skinFlags.push_back(1);
			}
			else {
				//�X�L���Ȃ��B
				mesh->skinFlags.push_back(0);
			}
		};
		//2. �C���f�b�N�X�o�b�t�@���쐬�B
		if (!tkmMesh.indexBuffer16Array.empty()) {
			//�C���f�b�N�X�̃T�C�Y��2byte
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer16Array.size());
			for (auto& tkIb : tkmMesh.indexBuffer16Array) {
				auto ib = new IndexBuffer;
				ib->Init(static_cast<int>(tkIb.indices.size()) * 2, 2);
				ib->Copy((uint16_t*)&tkIb.indices.at(0), 0, 0, 0);

				//�X�L�������邩�ǂ�����ݒ肷��B
				SetSkinFlag(tkIb.indices[0]);

				mesh->m_indexBufferArray.push_back(ib);
			}
		}
		else {
			//�C���f�b�N�X�̃T�C�Y��4byte
			mesh->m_indexBufferArray.reserve(tkmMesh.indexBuffer32Array.size());
			for (auto& tkIb : tkmMesh.indexBuffer32Array) {
				auto ib = new IndexBuffer;
				ib->Init(static_cast<int>(tkIb.indices.size()) * 4, 4);
				ib->Copy((uint32_t*)&tkIb.indices.at(0), 0, 0, 0);

				//�X�L�������邩�ǂ�����ݒ肷��B
				SetSkinFlag(tkIb.indices[0]);

				mesh->m_indexBufferArray.push_back(ib);
			}
		}
		//3. �}�e���A�����쐬�B
		auto& materialBank = GetMaterialBank();
		mesh->m_materials.reserve(tkmMesh.materials.size());
		for (auto& tkmMat : tkmMesh.materials) {
			char materiayKey[MAX_PATH];
			sprintf_s(
				materiayKey,
				MAX_PATH,
				"%s, %s, %s, %s, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s, %s, %s",
				fxFilePath,
				vsEntryPointFunc,
				vsSkinEntryPointFunc,
				psEntryPointFunc,
				(int)colorBufferFormat[0],
				(int)colorBufferFormat[1],
				(int)colorBufferFormat[2],
				(int)colorBufferFormat[3],
				(int)colorBufferFormat[4],
				(int)colorBufferFormat[5],
				(int)colorBufferFormat[6],
				(int)colorBufferFormat[7],
				alphaBlendMode,
				isDepthWrite,
				isDepthTest,
				cullMode,
				tkmMat.albedoMapFileName.empty() ? "none" : tkmMat.albedoMapFileName.c_str(),
				tkmMat.normalMapFileName.empty() ? "none" : tkmMat.normalMapFileName.c_str(),
				tkmMat.specularMapFileName.empty() ? "none" : tkmMat.specularMapFileName.c_str()
			);
			auto mat = materialBank.Get(materiayKey);
			if (mat == nullptr) {
				mat = new Material();

				mat->InitFromTkmMaterila(
					tkmMat,
					fxFilePath,
					vsEntryPointFunc,
					vsSkinEntryPointFunc,
					psEntryPointFunc,
					colorBufferFormat,
					NUM_SRV_ONE_MATERIAL,
					NUM_CBV_ONE_MATERIAL,
					NUM_CBV_ONE_MATERIAL * materialNum,
					NUM_SRV_ONE_MATERIAL * materialNum,
					alphaBlendMode,
					isDepthWrite,
					isDepthTest,
					cullMode
				);
				materialBank.Regist(materiayKey, mat);
			}
			//�쐬�����}�e���A�������J�E���g����B
			materialNum++;
			mesh->m_materials.push_back(mat);
		}

		m_meshs[meshNo] = mesh;

	}
	const VertexBuffer& MeshParts::GetAnimatedVertexBuffer(int meshNo)  const
	{
		return m_computedAnimationVertexBuffer->GetAnimatedVertexBuffer(meshNo);
	}
	VertexBuffer& MeshParts::GetAnimatedVertexBuffer(int meshNo)
	{
		return m_computedAnimationVertexBuffer->GetAnimatedVertexBuffer(meshNo);
	}
	const IndexBuffer& MeshParts::GetAnimatedIndexBuffer(int meshNo, int matNo) const
	{
		return m_computedAnimationVertexBuffer->GetAnimatedIndexBuffer(meshNo, matNo);
	}
	IndexBuffer& MeshParts::GetAnimatedIndexBuffer(int meshNo, int matNo)
	{
		return m_computedAnimationVertexBuffer->GetAnimatedIndexBuffer(meshNo, matNo);
	}
	void MeshParts::BindSkeleton(Skeleton& skeleton)
	{
		m_skeleton = &skeleton;
		//�\�����o�b�t�@���쐬����B
		m_boneMatricesStructureBuffer.Init(
			sizeof(Matrix),
			m_skeleton->GetNumBones(),
			m_skeleton->GetBoneMatricesTopAddress()
		);
	}
	void MeshParts::Draw(
		RenderContext& rc,
		const Matrix& mWorld,
		const Matrix& mView,
		const Matrix& mProj,
		int numInstance
	)
	{
		//���b�V�����ƂɃh���[
		//�v���~�e�B�u�̃g�|���W�[�̓g���C�A���O�����X�g�̂݁B
		rc.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�萔�o�b�t�@���X�V����B
		SConstantBuffer cb;
		cb.mWorld = mWorld;
		cb.mView = mView;
		cb.mProj = mProj;
		m_commonConstantBuffer.CopyToVRAM(cb);

		if (m_expandData) {
			m_expandConstantBuffer.CopyToVRAM(m_expandData);
		}
		if (m_expandData2) {
			m_expandConstantBuffer2.CopyToVRAM(m_expandData2);
		}
		if (m_boneMatricesStructureBuffer.IsInited()) {
			//�{�[���s����X�V����B
			m_boneMatricesStructureBuffer.Update(m_skeleton->GetBoneMatricesTopAddress());
		}
		int descriptorHeapNo = 0;
		int meshNo = 0;
		for (auto& mesh : m_meshs) {
			if (m_computedAnimationVertexBuffer){
				// ���_�o�b�t�@�̎��O�v�Z�������w�肳��Ă���̂ŁA�v�Z�ςݒ��_�o�b�t�@���擾����B
				rc.SetVertexBuffer(m_computedAnimationVertexBuffer->GetAnimatedVertexBuffer(meshNo));
			}
			else {
				rc.SetVertexBuffer(mesh->m_vertexBuffer);
			}
			
			//�}�e���A�����ƂɃh���[�B
			for (int matNo = 0; matNo < mesh->m_materials.size(); matNo++) {
				//���̃}�e���A�����\���Ă��郁�b�V���̕`��J�n�B
				mesh->m_materials[matNo]->BeginRender(
					rc, 
					mesh->skinFlags[matNo]
				);
				//2. �f�B�X�N���v�^�q�[�v��ݒ�B
				rc.SetDescriptorHeap(m_descriptorHeap);
				//3. �C���f�b�N�X�o�b�t�@��ݒ�B
				IndexBuffer* ib = nullptr;
				if (m_computedAnimationVertexBuffer) {
					ib = &m_computedAnimationVertexBuffer->GetAnimatedIndexBuffer(meshNo, matNo);
				}
				else {
					ib = mesh->m_indexBufferArray[matNo];
				}
				rc.SetIndexBuffer(*ib);
				//4. �h���[�R�[�������s�B
				rc.DrawIndexedInstance(ib->GetCount(), numInstance);
				descriptorHeapNo++;
			}
			meshNo++;
		}
	}
}