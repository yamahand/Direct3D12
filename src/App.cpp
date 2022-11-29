#include "App.h"

namespace {
	const auto ClassName = TEXT("SampleWindowClass");

	template<class T>
	void SafeRelease(T*& p) {
		if (p != nullptr) {
			p->Release();
			p = nullptr;
		}
	}
}

/// <summary>
/// �R���X�g���N�^
/// </summary>
/// <param name="width">�E�C���h�E�̕�</param>
/// <param name="height">�E�C���h�E�̍���</param>
App::App(uint32_t width, uint32_t height)
	: m_instance(nullptr)
	, m_hwnd(nullptr)
	, m_width(width)
	, m_height(height)
{
}

/// <summary>
/// �f�X�g���N�^
/// </summary>
App::~App()
{

}

/// <summary>
/// ���s
/// </summary>
void App::Run() {
	if (InitApp()) {
		MainLoop();
	}

	TermApp();
}

/// <summary>
/// ����������
/// </summary>
/// <returns>�������ɐ���������</returns>
bool App::InitApp() {
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

		// �f�o�b�O���C���[��L����
		if (SUCCEEDED(hr)) {
			debug->EnableDebugLayer();
		}
	}
#endif

	// �E�C���h�E�̏�����
	if (!InitWnd()) {
		return false;
	}

	if (!InitD3D()) {
		return false;
	}

	return true;
}

/// <summary>
/// �I������
/// </summary>
void App::TermApp() {
	TermD3D();
	TermWnd();
}

/// <summary>
/// �E�C���h�E�̏���������
/// </summary>
/// <returns></returns>
bool App::InitWnd() {
	auto hInstance = GetModuleHandle(nullptr);
	if (hInstance == nullptr)
		return false;

	// �E�C���h�E�ݒ�
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
	wc.hCursor = LoadCursor(hInstance, IDC_ARROW);
	wc.hbrBackground = GetSysColorBrush(COLOR_BACKGROUND);
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = ClassName;
	wc.hIconSm = LoadIcon(hInstance, IDI_APPLICATION);

	// �E�C���h�E�o�^
	if (!RegisterClassEx(&wc)) {
		return false;
	}

	// �C���X�^���X�n���h���ݒ�
	m_instance = hInstance;

	// �E�C���h�E�̃T�C�Y��ݒ�
	RECT rc = {};
	rc.right = static_cast<LONG>(m_width);
	rc.bottom = static_cast<LONG>(m_height);

	// �E�C���h�E�T�C�Y�𒲐�
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// �E�C���h�E�𐶐�
	m_hwnd = CreateWindowEx(
		0,
		ClassName,
		TEXT("Sample"),
		style,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		rc.right - rc.left,
		rc.bottom - rc.top,
		nullptr,
		nullptr,
		m_instance,
		nullptr
	);

	if (m_hwnd == nullptr) {
		return false;
	}

	// �E�C���h�E�\��
	ShowWindow(m_hwnd, SW_SHOWNORMAL);

	// �E�C���h�E���X�V
	UpdateWindow(m_hwnd);

	// �E�C���h�E�Ƀt�H�[�J�X��ݒ�
	SetFocus(m_hwnd);

	return true;
}

/// <summary>
/// �E�C���h�E�̏I������
/// </summary>
void App::TermWnd() {
	// �E�C���h�E�̓o�^������
	if (m_instance != nullptr) {
		UnregisterClass(ClassName, m_instance);
	}

	m_instance = nullptr;
	m_hwnd = nullptr;
}

bool App::InitD3D()
{
	// �f�o�C�X�̐���
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&m_pDevice)
	);

	if (FAILED(hr))
		return false;

	// �R�}���h�L���[�̐���
	{
		D3D12_COMMAND_QUEUE_DESC desc = {};
		desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
		desc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
		desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
		desc.NodeMask = 0;

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(&m_pQueue));
		if (FAILED(hr))
			return false;
	}

	// �X���b�v�`�F�C���̐���
	{
		// DXGI�t�@�N�g���[�̐���
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
			return false;

		// �X���b�v�`�F�C���̐ݒ�
		DXGI_SWAP_CHAIN_DESC desc = {};
		desc.BufferDesc.Width = m_width;
		desc.BufferDesc.Height = m_height;
		desc.BufferDesc.RefreshRate.Numerator = 60;
		desc.BufferDesc.RefreshRate.Denominator = 1;
		desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
		desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
		desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		desc.BufferCount = FrameCount;
		desc.OutputWindow = m_hwnd;
		desc.Windowed = TRUE;
		desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		desc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

		// �X���b�v�`�F�C���̐���
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			return false;
		}

		// IDXGISwapChain3���擾
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			SafeRelease(pSwapChain);
			return false;
		}

		// �o�b�N�o�b�t�@�C���f�b�N�X���擾
		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		SafeRelease(pFactory);
		SafeRelease(pSwapChain);
	}

	// �R�}���h�A���P�[�^�̐���
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCmdAllocator[i]));
			if (FAILED(hr)) {
				return false;
			}
		}
	}

	// �R�}���h���X�g�̐���
	{
		hr = m_pDevice->CreateCommandList(
			0,
			D3D12_COMMAND_LIST_TYPE_DIRECT,
			m_pCmdAllocator[m_frameIndex].Get(),
			nullptr,
			IID_PPV_ARGS(&m_pCmdList)
		);
		if (FAILED(hr)) {
			return false;
		}
	}

	// �����_�[�^�[�Q�b�g�r���[
	{
		// �f�B�X�N���v�^�q�[�v�̐ݒ�
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		// �f�B�X�N���v�^�q�[�v�𐶐�
		hr = m_pDevice->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_pHeapRTV));
		if (FAILED(hr)) {
			return false;
		}

		auto handle = m_pHeapRTV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

		for (auto i = 0; i < FrameCount; ++i)
		{
			hr = m_pSwapChain->GetBuffer(i, IID_PPV_ARGS(&m_pColorBuffer[i]));
			if (FAILED(hr)) {
				return false;
			}

			D3D12_RENDER_TARGET_VIEW_DESC viewDesc = {};
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			// �����_�[�^�[�Q�b�g�r���[�̐���
			m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

			m_handleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	// �t�F���X�̐���
	{
		// �t�F���X�J�E���^�[�����Z�b�g
		for (auto i = 0; i < FrameCount; ++i)
		{
			m_fenceCounter[i] = 0;
		}
		// �t�F���X�̐���
		hr = m_pDevice->CreateFence(
			m_fenceCounter[m_frameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_pFence)
		);
		if (FAILED(hr)) {
			return false;
		}

		++m_fenceCounter[m_frameIndex];

		// �C�x���g�̐���
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr) {
			return false;
		}
	}

	// �R�}���h�����
	m_pCmdList->Close();

	return true;
}

void App::TermD3D()
{
	// GPU�����̊�����ҋ@
	WaitGpu();

	// �C�x���g�j��
	if (m_fenceEvent != nullptr) {
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	// �t�F���X�j��
	m_pFence.Reset();

	// �����_�[�^�[�Q�b�g�r���[�̔j��
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i].Reset();
	}

	// �R�}���h���X�g�̔j��
	m_pCmdList.Reset();

	// �R�}���h�A���P�[�^�̔j��
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pCmdAllocator[i].Reset();
	}

	// �X���b�v�`�F�C���̔j��
	m_pSwapChain.Reset();

	// �R�}���h�L���[�̔j��
	m_pQueue.Reset();

	// �f�o�C�X�͔j��
	m_pDevice.Reset();

}

void App::Render()
{
	// �R�}���h�̋L�^�J�n
	m_pCmdAllocator[m_frameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_frameIndex].Get(), nullptr);

	// ���\�[�X�o���A�̐ݒ�
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// ���\�[�X�o���A
	m_pCmdList->ResourceBarrier(1, &barrier);

	// �����_�[�^�[�Q�b�g�̐ݒ�
	m_pCmdList->OMSetRenderTargets(1, &m_handleRTV[m_frameIndex], FALSE, nullptr);

	// �N���A�J���[�ݒ�
	float clearColor[] = {0.25f, 0.25f, 0.25f, 1.0f};

	// �����_�[�^�[�Q�b�g�r���[��kuria 
	m_pCmdList->ClearRenderTargetView(m_handleRTV[m_frameIndex], clearColor, 0, nullptr);

	// �`�揈��
	{
		// todo: �|���S���`��
	}

	// ���\�[�X�o���A�ݒ�
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// ���\�[�X�o���A
	m_pCmdList->ResourceBarrier(1, &barrier);

	// �R�}���h�L�^���I��
	m_pCmdList->Close();

	// �R�}���h�����s
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get()};
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// ��ʂɕ\��
	Present(1);
}

void App::WaitGpu()
{
	// �V�O�i������
	m_pQueue->Signal(m_pFence.Get(), m_fenceCounter[m_frameIndex]);

	// �������ɃC�x���g��ݒ肷��
	m_pFence->SetEventOnCompletion(m_fenceCounter[m_frameIndex], m_fenceEvent);

	// �ҋ@����
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	++m_fenceCounter[m_frameIndex];
}

void App::Present(uint32_t interval)
{
	// ��ʂɕ\��
	m_pSwapChain->Present(interval, 0);

	// �V�O�i������
	const auto currentValue = m_fenceCounter[m_frameIndex];
	m_pQueue->Signal(m_pFence.Get(), currentValue);

	// �o�b�N�o�b�t�@�ԍ����X�V
	m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// ���̃t���[���̕`�揀�����ł��Ă��Ȃ��Ȃ�ҋ@����
	if (m_pFence->GetCompletedValue() < m_fenceCounter[m_frameIndex]) {
		m_pFence->SetEventOnCompletion(m_fenceCounter[m_frameIndex], m_fenceEvent);
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// ���̃t���[���̃t�F���X�J�E���^�[�𑝂₷
	m_fenceCounter[m_frameIndex] = currentValue + 1;
}

/// <summary>
/// ���C�����[�v
/// </summary>
void App::MainLoop() {
	MSG msg = {};

	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE) == TRUE) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		Render();
	}
}

LRESULT CALLBACK App::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	switch (msg)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}
