#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <ComPtr.h>
#include <cstdint>

class DescriptorHandle;
class DescriptorPool;

class ColorTarget {
public:
	ColorTarget();
	~ColorTarget();

	ColorTarget(const ColorTarget&) = delete;
	void operator = (const ColorTarget&) = delete;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="pPoolRTV">�f�B�X�N���v�^�[�v�[��(RTV�p)</param>
	/// <param name="width">����</param>
	/// <param name="height">����</param>
	/// <param name="format">�s�N�Z���t�H�[�}�b�g</param>
	/// <param name="useSRGB">SRGB���g�p���邩</param>
	/// <returns>�������ɐ���������true</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, uint32_t width, uint32_t height, DXGI_FORMAT format, bool useSRGB);

	/// <summary>
	/// �o�b�N�o�b�t�@���g���ď�����
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="pPoolRTV">�f�B�X�N���v�^�v�[��(RTV�p)</param>
	/// <param name="useSRGB">SRGB���g�p���邩</param>
	/// <param name="index">�o�b�N�o�b�t�@�C���f�b�N�X</param>
	/// <param name="pSwapChain">�X���b�v�`�F�C��</param>
	/// <returns>�������ɐ���������true</returns>
	bool InitFromBackBuffer(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, bool useSRGB, uint32_t index, IDXGISwapChain* pSwapChain);

	/// <summary>
	/// �I������
	/// </summary>
	void Term();

	/// <summary>
	/// �f�B�X�N���v�^�n���h��(RTV�p)���擾
	/// </summary>
	/// <returns>�f�B�X�N���v�^�n���h��(RTV�p)</returns>
	DescriptorHandle* GetHandleRTV() const;

	/// <summary>
	///  ���\�[�X�擾
	/// </summary>
	/// <returns>���\�[�X</returns>
	ID3D12Resource* GetResource() const;

	/// <summary>
	/// ���\�[�X�ݒ���擾
	/// </summary>
	/// <returns>���\�[�X�ݒ�</returns>
	D3D12_RESOURCE_DESC GetDesc() const;

	/// <summary>
	/// �����_�[�^�[�Q�b�g�r���[�̐ݒ���擾
	/// </summary>
	/// <returns>�����_�[�^�[�Q�b�g�r���[�̐ݒ�</returns>
	D3D12_RENDER_TARGET_VIEW_DESC GetViewDesc() const;

private:
	ComPtr<ID3D12Resource> m_pTarget = nullptr;
	DescriptorHandle* m_pHandleRTV = nullptr;
	DescriptorPool* m_pPoolRTV = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC m_viewDesc = {};

};
