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
	VertexBuffer m_vb = {};				//!< 頂点バッファ
	IndexBuffer m_ib = {};				//!< インデックスバッファ
	uint32_t m_materialId = UINT32_MAX;	//!< マテリアルID
	uint32_t m_indexCount = 0;			//!< インデックス数
};
