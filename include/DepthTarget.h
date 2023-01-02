#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <cstdint>

class DescriptorHandle;
class DescriptorPool;

class DepthTarget {
public:
	DepthTarget();
	~DepthTarget();

	DepthTarget(const DepthTarget&) = delete;
	void operator = (const DepthTarget&) = delete;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="pPoolDSV">�f�B�X�N���v�^�[�v�[��(DSV�p)</param>
	/// <param name="width">����</param>
	/// <param name="height">����</param>
	/// <param name="format">�s�N�Z���t�H�[�}�b�g</param>
	/// <returns>�������ɐ���������true</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV, uint32_t width, uint32_t height, DXGI_FORMAT format);

	/// <summary>
	/// �I������
	/// </summary>
	void Term();

	/// <summary>
	/// �f�B�X�N���v�^�n���h��(DSV�p)���擾
	/// </summary>
	/// <returns>�f�B�X�N���v�^�n���h��(DSV�p)</returns>
	DescriptorHandle* GetHandleDSV() const;

	/// <summary>
	/// ���\�[�X���擾
	/// </summary>
	/// <returns>���\�[�X</returns>
	ID3D12Resource* GetResource() const;

	/// <summary>
	/// ���\�[�X�ݒ���擾
	/// </summary>
	/// <returns>���\�[�X�ݒ�</returns>
	D3D12_RESOURCE_DESC GetDesc() const;

	/// <summary>
	/// �[�x�X�e���V���r���[�̐ݒ���擾
	/// </summary>
	/// <returns>�[�x�X�e���V���r���[�̐ݒ�</returns>
	D3D12_DEPTH_STENCIL_VIEW_DESC GetViewDesc() const;

private:
	ComPtr<ID3D12Resource>	m_pTarget = nullptr;
	DescriptorHandle* m_pHandleDSV = nullptr;
	DescriptorPool* m_pPoolDSV = nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC m_viewDesc = {};

};
