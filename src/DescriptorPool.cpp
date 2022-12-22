#include "DescriptorPool.h"

DescriptorPool::DescriptorPool()
	:m_refCount(1)
	, m_pool()
	, m_pHeap()
	, m_descriptorSize(0)
{
}

DescriptorPool::~DescriptorPool() {
	m_pool.Term();
	m_pHeap.Reset();
	m_descriptorSize = 0;
}

void DescriptorPool::AddRef() {
	++m_refCount;
}

void DescriptorPool::Release() {
	--m_refCount;
	if (m_refCount == 0) {
		delete this;
	}
}

uint32_t DescriptorPool::GetCount() const {
	return m_refCount;
}

DescriptorHandle* DescriptorPool::AllocHandle() {
	// �������֐�
	auto func = [&](uint32_t index, DescriptorHandle* pHandle) {
		auto handleCPU = m_pHeap->GetCPUDescriptorHandleForHeapStart();
		handleCPU.ptr += m_descriptorSize * index;

		auto handleGPU = m_pHeap->GetGPUDescriptorHandleForHeapStart();
		handleGPU.ptr = m_descriptorSize * index;

		pHandle->handleCPU = handleCPU;
		pHandle->handleGPU = handleGPU;
	};

	return m_pool.Alloc(func);
}

void DescriptorPool::FreeHandle(DescriptorHandle*& pHandle) {
	if (pHandle != nullptr) {
		// �n���h�����v�[���ɖ߂�
		m_pool.Free(pHandle);
		pHandle = nullptr;
	}
}

uint32_t DescriptorPool::GetAvailableHandleCount() const {
	return m_pool.GetAvailableCount();
}

uint32_t DescriptorPool::GetAllocatedHandle() const {
	return m_pool.GetUsedCount();
}

uint32_t DescriptorPool::GetHandleCount() const {
	return m_pool.GetSize();
}

ID3D12DescriptorHeap* const DescriptorPool::GetHeap() const {
	return m_pHeap.Get();
}

bool DescriptorPool::Create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, DescriptorPool** ppPool) {
	if (pDevice == nullptr || pDesc == nullptr || ppPool == nullptr) {
		return false;
	}

	// �C���X�^���X�𐶐�
	auto instance = new (std::nothrow) DescriptorPool();
	if (instance == nullptr) {
		return false;
	}

	// �f�B�X�N���v�^�q�[�v�𐶐�
	auto hr = pDevice->CreateDescriptorHeap(pDesc, IID_PPV_ARGS(instance->m_pHeap.GetAddressOf()));
	if (FAILED(hr)) {
		instance->Release();
		return false;
	}

	// �v�[����������
	if (!instance->m_pool.Init(pDesc->NumDescriptors)) {
		instance->Release();
		return false;
	}

	// �f�B�X�N���v�^�̉��Z�T�C�Y���擾
	instance->m_descriptorSize = pDevice->GetDescriptorHandleIncrementSize(pDesc->Type);

	// �C���X�^���X��ݒ�
	*ppPool = instance;

	// ����I��
	return true;
}
