#include "DepthTarget.h"
#include "DescriptorPool.h"

DepthTarget::DepthTarget()
{
}

DepthTarget::~DepthTarget()
{
	Term();
}

bool DepthTarget::Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV, uint32_t width, uint32_t height, DXGI_FORMAT format)
{
	if (pDevice == nullptr || pPoolDSV == nullptr || width == 0 || height == 0) {
		return false;
	}

	assert(m_pHandleDSV == nullptr);
	assert(m_pPoolDSV == nullptr);

	m_pPoolDSV = pPoolDSV;
	m_pPoolDSV->AddRef();

	m_pHandleDSV = m_pPoolDSV->AllocHandle();
	if (m_pHandleDSV == nullptr) {
		return false;
	}

	D3D12_HEAP_PROPERTIES prop = {};
	prop.Type = D3D12_HEAP_TYPE_DEFAULT;
	prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	prop.CreationNodeMask = 1;
	prop.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC desc = {};
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Alignment = 0;
	desc.Width = UINT64(width);
	desc.Height = height;
	desc.Format = format;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE clearValue;
	clearValue.Format = format;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	auto hr = pDevice->CreateCommittedResource(&prop, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(m_pTarget.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		return false;
	}

	m_viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	m_viewDesc.Format = format;
	m_viewDesc.Texture2D.MipSlice = 0;
	m_viewDesc.Flags = D3D12_DSV_FLAG_NONE;

	pDevice->CreateDepthStencilView(m_pTarget.Get(), &m_viewDesc, m_pHandleDSV->handleCPU);

	return true;
}

void DepthTarget::Term()
{
	m_pTarget.Reset();

	if (m_pPoolDSV != nullptr && m_pHandleDSV != nullptr) {
		m_pPoolDSV->FreeHandle(m_pHandleDSV);
		m_pHandleDSV = nullptr;
	}

	if (m_pPoolDSV != nullptr) {
		m_pPoolDSV->Release();
		m_pPoolDSV = nullptr;
	}
}

DescriptorHandle* DepthTarget::GetHandleDSV() const
{
	return m_pHandleDSV;
}

ID3D12Resource* DepthTarget::GetResource() const
{
	return m_pTarget.Get();
}

D3D12_RESOURCE_DESC DepthTarget::GetDesc() const
{
	if (m_pTarget == nullptr) {
		return D3D12_RESOURCE_DESC();
	}
	return m_pTarget->GetDesc();
}

D3D12_DEPTH_STENCIL_VIEW_DESC DepthTarget::GetViewDesc() const
{
	return m_viewDesc;
}
