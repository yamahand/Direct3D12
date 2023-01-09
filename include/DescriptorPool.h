#pragma once

#include <d3d12.h>
#include <atomic>
#include <ComPtr.h>
#include <Pool.h>

class DescriptorHandle {
public :
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = { 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = { 0 };

	bool HasCPU() const { return handleCPU.ptr != 0; }
	bool HasGPU() const { return handleGPU.ptr != 0; }
};

/// <summary>
/// �f�B�X�N���v�^�[�e�[�u��
/// </summary>
class DescriptorPool{
private:
	DescriptorPool();

	~DescriptorPool();

	DescriptorPool(const DescriptorPool&) = delete;
	void operator = (const DescriptorPool&) = delete;

public:
	/// <summary>
	/// �f�B�X�N���v�^�[�e�[�u������
	/// </summary>
	/// <param name="pDevice"></param>
	/// <param name="pDesc"></param>
	/// <param name="ppPool"></param>
	/// <returns></returns>
	static bool Create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, DescriptorPool** ppPool);

	/// <summary>
	/// �Q�ƃJ�E���g�̒ǉ�
	/// </summary>
	void AddRef();

	/// <summary>
	/// �J������
	/// </summary>
	void Release();

	/// <summary>
	/// �Q�ƃJ�E���g���̎擾
	/// </summary>
	/// <returns>�Q�ƃJ�E���g��</returns>
	uint32_t GetCount() const;

	/// <summary>
	/// �f�B�X�N���v�^�n���h�������蓖��
	/// </summary>
	/// <returns>���蓖�Ă�ꂽ�f�B�X�N���v�^�n���h��</returns>
	DescriptorHandle* AllocHandle();

	/// <summary>
	/// �f�B�X�N���v�^�n���h�����J������
	/// </summary>
	/// <param name="pHandle">�J������f�B�X�N���v�^�n���h��</param>
	void FreeHandle(DescriptorHandle*& pHandle);

	/// <summary>
	/// ���p�\�ȃn���h�������擾
	/// </summary>
	/// <returns>���p�\�ȃn���h����</returns>
	uint32_t GetAvailableHandleCount() const;

	/// <summary>
	/// ���蓖�čς݃n���h�������擾
	/// </summary>
	/// <returns>���蓖�čς݃n���h����</returns>
	uint32_t GetAllocatedHandle() const;

	/// <summary>
	/// �n���h���������擾
	/// </summary>
	/// <returns>�n���h������</returns>
	uint32_t GetHandleCount() const;

	/// <summary>
	/// �f�B�X�N���v�^�q�[�v���擾
	/// </summary>
	/// <returns>�f�B�X�N���v�^�q�[�v</returns>
	ID3D12DescriptorHeap* const GetHeap() const;

private:
	std::atomic<uint32_t> m_refCount;	//!< �Q�ƃJ�E���g
	Pool<DescriptorHandle>	m_pool;	//!< �f�B�X�N���v�^�n���h���v�[��
	ComPtr<ID3D12DescriptorHeap>	m_pHeap;	//!< �f�B�X�N���v�^�q�[�v
	uint32_t m_descriptorSize;	//!< �f�B�X�N���v�^�T�C�Y
};
