#include "Material.h"
#include "FileUtil.h"

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
	for (auto& itr : m_pTexture){
		if (itr.second != nullptr) {
			itr.second->Term();
			delete itr.second;
			itr.second = nullptr;
		}
	}

	for (size_t i = 0; i < m_subset.size(); i++)
	{
		if (m_subset[i].pConstantBuffer != nullptr) {
			m_subset[i].pConstantBuffer->Term();
			delete m_subset[i].pConstantBuffer;
			m_subset[i].pConstantBuffer = nullptr;
		}
	}

	m_pTexture.clear();
	m_subset.clear();

	if (m_pDevice != nullptr) {
		m_pDevice->Release();
		m_pDevice = nullptr;
	}

	if (m_pPool != nullptr) {
		m_pPool->Release();
		m_pPool = nullptr;
	}
}

bool Material::SetTexture(size_t index, TEXTURE_USAGE usage, const std::wstring& path, DirectX::ResourceUploadBatch& batch)
{
	// 範囲内チェック
	if (index >= GetCount()) {
		return false;
	}

	// 登録済みかチェック
	if (m_pTexture.find(path) != m_pTexture.end()) {
		m_subset[index].textureHandle[usage] = m_pTexture[path]->GetHandleGPU();
		return true;
	}

	// ファイルパスが存在するかチェック
	std::wstring findPath;
	if (!SearchFilePath(path.c_str(), findPath)) {
		// 存在しない場合はダミーテクスチャ設定
		m_subset[index].textureHandle[usage] = m_pTexture[DummyTag]->GetHandleGPU();
		return true;
	}

	// ファイル名であることをチェック
	if (PathIsDirectoryW(findPath.c_str()) != FALSE) {
		m_subset[index].textureHandle[usage] = m_pTexture[DummyTag]->GetHandleGPU();
		return true;
	}

	// インスタンス生成
	auto pTexture = new (std::nothrow)Texture();
	if (pTexture == nullptr) {
		return false;
	}

	// 初期化
	if (!pTexture->Init(m_pDevice, m_pPool, findPath.c_str(), batch)) {
		pTexture->Term();
		delete pTexture;
		return false;
	}

	// 登録
	m_pTexture[path] = pTexture;
	m_subset[index].textureHandle[usage] = pTexture->GetHandleGPU();

	return true;
}

void* Material::GetBufferPtr(size_t index) const {
	if (index >= GetCount()) {
		return nullptr;
	}

	return m_subset[index].pConstantBuffer->GetPtr();
}

D3D12_GPU_VIRTUAL_ADDRESS Material::GetBufferAddress(size_t index) const
{
	if (index >= GetCount()) {
		return D3D12_GPU_VIRTUAL_ADDRESS();
	}

	return m_subset[index].pConstantBuffer->GetAddress();
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetTextureHandle(size_t index, TEXTURE_USAGE usage) const
{
	if (index >= GetCount()) {
		return D3D12_GPU_DESCRIPTOR_HANDLE();
	}

	return m_subset[index].textureHandle[usage];
}

size_t Material::GetCount() const
{
	return m_subset.size();
}
