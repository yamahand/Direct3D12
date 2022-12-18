#pragma once

#include <d3d12.h>
#include <atomic>
#include <ComPtr.h>
#include <Pool.h>

class DescriptorHandle {
public :
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;

	bool HasCPU() const { return handleCPU.ptr != 0; }
	bool HasGPU() const { return handleGPU.ptr != 0; }
};

/// <summary>
/// �f�B�X�N���v�^�[�e�[�u��
/// </summary>
class DescriptorPool{
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
	void AddRed();

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
};
