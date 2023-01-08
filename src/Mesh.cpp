#include "Mesh.h"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
	Term();
}

bool Mesh::Init(ID3D12Device* pDevice, const ResMesh& resource)
{
	if (pDevice == nullptr) {
		return false;
	}

	if (!m_vb.Init(pDevice, sizeof(MeshVertex) * resource.vertices.size(), resource.vertices.data())) {
		return false;
	}

	if (!m_ib.Init(pDevice, sizeof(MeshVertex) * resource.indices.size(), resource.indices.data())) {
		return false;
	}

	m_materialId = resource.materialId;
	m_indexCount = uint32_t(resource.indices.size());

	return true;
}

void Mesh::Term()
{
	m_vb.Term();
	m_ib.Term();
	m_materialId = UINT32_MAX;
	m_indexCount = 0;
}

void Mesh::Draw(ID3D12GraphicsCommandList* pCmdList)
{
	auto vbv = m_vb.GetView();
	auto ibv = m_ib.GetView();
	pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	pCmdList->IASetVertexBuffers(0, 1, &vbv);
	pCmdList->IASetIndexBuffer(&ibv);
	pCmdList->DrawIndexedInstanced(m_indexCount, 1, 0, 0, 0);
}

uint32_t Mesh::GetMaterialId() const
{
	return m_materialId;
}
