#pragma once

#include <d3d12.h>
#include <ComPtr.h>

class Fence {
public:
	/// <summary>
	/// �R���X�g���N�^
	/// </summary>
	Fence();

	/// <summary>
	/// �f�X�g���N�^
	/// </summary>
	~Fence();

	Fence(const Fence&) = delete;
	void operator = (const Fence&) = delete;

	/// <summary>
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <returns>����������������true</returns>
	bool Init(ID3D12Device* pDevice);

	/// <summary>
	/// �I������
	/// </summary>
	void Term();

	/// <summary>
	/// �V�O�i����ԂɂȂ�܂Ŏw�莞�ԑҋ@
	/// </summary>
	/// <param name="pQueue">�R�}���h�L���[</param>
	/// <param name="timeout">�^�C���A�E�g����</param>
	void Wait(ID3D12CommandQueue* pQueue, UINT timeout);

	/// <summary>
	/// �V�O�i����ԂɂȂ�܂őҋ@
	/// </summary>
	/// <param name="pQueue">�R�}���h�L���[</param>
	void Sync(ID3D12CommandQueue* pQueue);

private:
	ComPtr<ID3D12Fence>	m_pFence = nullptr;
	HANDLE m_event = {};
	UINT m_counter = 0;
};
