#include "Fence.h"

Fence::Fence()
{
}

Fence::~Fence()
{
	Term();
}

bool Fence::Init(ID3D12Device* pDevice)
{
	if (pDevice == nullptr) {
		return false;
	}

	m_event = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
	if (m_event == nullptr) {
		return false;
	}

	auto hr = pDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(m_pFence.ReleaseAndGetAddressOf()));
	if (FAILED(hr)) {
		return false;
	}

	m_counter = 1;

	return true;
}

void Fence::Term()
{
	if (m_event != nullptr) {
		CloseHandle(m_event);
		m_event = nullptr;
	}

	m_pFence.Reset();

	m_counter = 0;
}

void Fence::Wait(ID3D12CommandQueue* pQueue, UINT timeout)
{
	if (pQueue == nullptr) {
		return;
	}

	const auto fenceValue = m_counter;

	// �V�O�i������
	auto hr = pQueue->Signal(m_pFence.Get(), fenceValue);
	if (FAILED(hr)) {
		return;
	}

	// �J�E���^�[�𑝂₷
	m_counter++;

	// ���t���[���̕`�揀�����܂��ł���Αҋ@����
	if (m_pFence->GetCompletedValue() < fenceValue) {
		// �������ɃC�x���g��ݒ�
		auto hr = m_pFence->SetEventOnCompletion(fenceValue, m_event);
		if (FAILED(hr)) {
			return;
		}

		// �ҋ@����
		if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_event, timeout, FALSE)) {
			return;
		}
	}
}

void Fence::Sync(ID3D12CommandQueue* pQueue)
{
	if (pQueue == nullptr) {
		return;
	}

	// �V�O�i������
	auto hr = pQueue->Signal(m_pFence.Get(), m_counter);
	if (FAILED(hr)) {
		return;
	}

	// �������ɃC�x���g��ݒ�
	hr = m_pFence->SetEventOnCompletion(m_counter, m_event);
	if (FAILED(hr)) {
		return;
	}

	// �ҋ@����
	if (WAIT_OBJECT_0 != WaitForSingleObjectEx(m_event, INFINITE, FALSE)) {
		return;
	}

	// �J�E���^�[�𑝂₷
	++m_counter;
}
