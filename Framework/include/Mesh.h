#pragma once

#include <ResMesh.h>
#include <VertexBuffer.h>
#include <IndexBuffer.h>

class Mesh {
public:
	Mesh();
	virtual ~Mesh();

	Mesh(const Mesh&) = delete;
	void operator = (const Mesh&) = delete;

	bool Init(ID3D12Device* pDevice, const ResMesh& resource);

	void Term();

	void Draw(ID3D12GraphicsCommandList* pCmdList);

	uint32_t GetMaterialId() const;

private:
	VertexBuffer m_vb = {};				//!< ���_�o�b�t�@
	IndexBuffer m_ib = {};				//!< �C���f�b�N�X�o�b�t�@
	uint32_t m_materialId = UINT32_MAX;	//!< �}�e���A��ID
	uint32_t m_indexCount = 0;			//!< �C���f�b�N�X��
};
