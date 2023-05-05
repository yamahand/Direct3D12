#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <ResourceUploadBatch.h>

class DescriptorHandle;
class DescriptorPool;

class Texture {
public:
	Texture();
	~Texture();

	Texture(const Texture&) = delete;
	void operator = (const Texture&) = delete;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
	/// <param name="fileName">�t�@�C����</param>
	/// <param name="useSRGB">SRGB���g�p���邩</param>
	/// <param name="batch">�A�b�v���[�h�o�b�`</param>
	/// <returns>����������true</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, const wchar_t* fileName, bool useSRGB, DirectX::ResourceUploadBatch& batch);

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="pPool">�f�B�X�N���v�^�v�[��</param>
	/// <param name="pDesc">�ŃX�N���v�^</param>
	/// <param name="useSRGB">SRGB���g�p���邩</param>
	/// <param name="isCube">�L���[�u�}�b�v��</param>
	/// <returns>����������true</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, const D3D12_RESOURCE_DESC* pDesc, bool useSRGB, bool isCube);

	/// <summary>
	/// �I������
	/// </summary>
	void Term();

	/// <summary>
	/// CPU�f�B�X�N���v�^�n���h���擾
	/// </summary>
	/// <returns>CPU�f�B�X�N���v�^�n���h��</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU();

	/// <summary>
	/// GPU�f�B�X�N���v�^�n���h���擾
	/// </summary>
	/// <returns>CPU�f�B�X�N���v�^�n���h��</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU();

private:
	/// <summary>
	/// �V�F�[�_���\�[�X�r���[�̐ݒ���擾
	/// </summary>
	/// <param name="isCube">�L���[�u�}�b�v��</param>
	/// <returns>�V�F�[�_�[���\�[�X�r���[�̐ݒ�</returns>
	D3D12_SHADER_RESOURCE_VIEW_DESC GetViewDesc(bool isCube);

private:
	ComPtr<ID3D12Resource> m_pTex;
	DescriptorHandle* m_pHandle;
	DescriptorPool* m_pPool;
};
