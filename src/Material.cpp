#include "Material.h"

namespace {
	constexpr const wchar_t* DummyTag = L"";
}

Material::Material() {

}

Material::~Material() {
	Term();
}

bool Material::Init(ID3D12Device* pDevice, DescriptorPool* pPool, size_t bufferSize, size_t count)
{
	if (pDevice == nullptr || pPool == nullptr || count == 0) {
		return false;
	}

	Term();

	m_pDevice = pDevice;
	m_pDevice->AddRef();

	m_pPool = pPool;
	m_pPool->AddRef();

	m_subset.resize(count);

	// ダミーテクスチャ
	{
		auto pTexture = new (std::nothrow) Texture();
		if (pTexture == nullptr) {
			return false;
		}

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = 1;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		if (!pTexture->Init(pDevice, pPool, &desc, false)) {
			pTexture->Term();
			delete pTexture;
			return false;
		}

		m_pTexture[DummyTag] = pTexture;
	}

	auto size = bufferSize * count;
	if (size > 0) {
		for (size_t i = 0; i < m_subset.size(); i++)
		{
			auto pBuffer = new (std::nothrow) ConstantBuffer();
			if (pBuffer == nullptr) {
				return false;
			}
			if (!pBuffer->Init(pDevice, pPool, bufferSize)) {
				return false;
			}

			m_subset[i].pConstantBuffer = pBuffer;
			for (auto j = 0; j < TEXTURE_USAGE_COUNT; j++)
			{
				m_subset[i].textureHandle[j].ptr = 0;
			}
		}
	}
	else {
		for (size_t i = 0; i < m_subset.size(); i++)
		{
			m_subset[i].pConstantBuffer = nullptr;
			for (auto j = 0; j < TEXTURE_USAGE_COUNT; j++)
			{
				m_subset[i].textureHandle[j].ptr = 0;
			}
		}
	}
	return true;
}

void Material::Term()
{

}

bool Material::SetTexture(size_t index, TEXTURE_USAGE usage, const std::wstring& path, DirectX::ResourceUploadBatch& batch)
{

}

void* Material::GetBufferPtr(size_t index) const {
}

D3D12_GPU_VIRTUAL_ADDRESS Material::GetBufferAddress(size_t index) const
{

}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetTextureHandle(size_t index, TEXTURE_USAGE usage) const
{

}

size_t Material::GetCount() const
{

}
