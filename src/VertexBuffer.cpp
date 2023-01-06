#include "VertexBuffer.h"

VertexBuffer::VertexBuffer()
{
}

VertexBuffer::~VertexBuffer()
{
	Term();
}

bool VertexBuffer::Init(ID3D12Device* pDevice, size_t size, size_t stride, const void* pInitData)
{
	if (pDevice == nullptr || size == 0 || stride == 0) {
		return false;
	}

	// �q�[�v�v���p�e�B
	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_UPLOAD;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	// ���\�[�X�ݒ�
	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Alignment = 0;
	desc.Width = UINT64(size);
	desc.Height = 1;
	desc.DepthOrArraySize = 1;
	desc.MipLevels = 1;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.Flags = D3D12_RESOURCE_FLAG_NONE;

	// ���\�[�X�𐶐�
	auto hr = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(m_pVB.GetAddressOf()));
	if (FAILED(hr)) {
		return false;
	}

	// ���_�o�b�t�@�r���[�ݒ�
	m_view.BufferLocation = m_pVB->GetGPUVirtualAddress();
	m_view.StrideInBytes = UINT(stride);
	m_view.SizeInBytes = UINT(size);

	// �������f�[�^������Ώ�������
	if (pInitData != nullptr) {
		void* ptr = Map();
		if (ptr == nullptr) {
			return false;
		}

		memcpy(ptr, pInitData, size);

		m_pVB->Unmap(0, nullptr);
	}

	return true;
}

void VertexBuffer::Term()
{
	m_pVB.Reset();
	memset(&m_view, 0, sizeof(m_view));
}

void* VertexBuffer::Map()
{
	void* ptr;
	auto hr = m_pVB->Map(0, nullptr, &ptr);
	if (FAILED(hr)) {
		return nullptr;
	}
	return ptr;
}

void VertexBuffer::Unma()
{
	m_pVB->Unmap(0, nullptr);
}

D3D12_VERTEX_BUFFER_VIEW VertexBuffer::GetView() const
{
	return m_view;
}
