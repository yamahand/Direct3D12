#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <cstdint>
#include <vector>


class CommandList {
public:

	CommandList();
	~CommandList();

	CommandList(const CommandList&) = delete;
	void operator = (const CommandList&) = delete;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="type">�R�}���h���X�g�^�C�v</param>
	/// <param name="count">�A���P�[�^��.�_�u���o�b�t�@�̎���2��ݒ�</param>
	/// <returns>�������ɐ���������true</returns>
	bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t count);

	/// <summary>
	/// �I������
	/// </summary>
	void Term();

	/// <summary>
	/// ���Z�b�g�������s�����R�}���h���X�g���擾
	/// </summary>
	/// <returns>���Z�b�g�������s�����R�}���h���X�g</returns>
	ID3D12GraphicsCommandList* Reset();

private:
	ComPtr<ID3D12GraphicsCommandList>	m_pCmdList = nullptr;
	std::vector<ComPtr<ID3D12CommandAllocator>> m_pAllocators = {};
	uint32_t m_index = 0;
};
