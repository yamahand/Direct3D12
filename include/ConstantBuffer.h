#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <vector>

class DescriptorHandle;
class DescriptorPool;

class ConstantBuffer
{
public:
	ConstantBuffer();
	~ConstantBuffer();

private:
	ConstantBuffer(const ConstantBuffer&) = delete;
	void operator = (const ConstantBuffer&) = delete;

public:
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, size_t size);

	void Term();

	D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const;

	D3D12_CPU_DESCRIPTOR_HANDLE GethandleCPU() const;

	D3D12_GPU_DESCRIPTOR_HANDLE GethandleGPU() const;

	void* GetPtr() const;

	template<typename T>
	T* GetPtr() const {
		return reinterpret_cast<T*>(GetPtr());
	}

private:
	ComPtr<ID3D12Resource> m_pCB = {};
	DescriptorHandle* m_pHandle = nullptr;
	DescriptorPool* m_pPool = nullptr;
	D3D12_CONSTANT_BUFFER_VIEW_DESC m_desc = {};
	void* m_pMappedPtr = nullptr;
};
