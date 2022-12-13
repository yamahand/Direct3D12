#include "App.h"
#include "ResourceUploadBatch.h"
#include "DDSTextureLoader.h"
#include "VertexTypes.h"
#include "Mesh.h"
#include <cassert>

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

	if (!OnInit()) {
		return false;
	}

	return true;
}

/// <summary>
/// �I������
/// </summary>
void App::TermApp() {
	OnTerm();
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
			viewDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
			viewDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
			viewDesc.Texture2D.MipSlice = 0;
			viewDesc.Texture2D.PlaneSlice = 0;

			// �����_�[�^�[�Q�b�g�r���[�̐���
			m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

			m_handleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	// �f�v�X�X�e���V���o�b�t�@�̍쐬
	{
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_DEFAULT;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Alignment = 0;
		desc.Width = m_width;
		desc.Height = m_height;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

		D3D12_CLEAR_VALUE clearValue = {};
		clearValue.Format = DXGI_FORMAT_D32_FLOAT;
		clearValue.DepthStencil.Depth = 1.0f;
		clearValue.DepthStencil.Stencil = 0;

		hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&clearValue,
			IID_PPV_ARGS(m_pDepthBuffer.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			return false;
		}

		// �f�B�X�N���v�^�q�[�v�̐ݒ�
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		heapDesc.NodeMask = 0;

		hr = m_pDevice->CreateDescriptorHeap(
			&heapDesc,
			IID_PPV_ARGS(m_pHeapDSV.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			return false;
		}

		auto handle = m_pHeapDSV->GetCPUDescriptorHandleForHeapStart();
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);

		D3D12_DEPTH_STENCIL_VIEW_DESC viewDesc = {};
		viewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		viewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		viewDesc.Texture2D.MipSlice = 0;
		viewDesc.Flags = D3D12_DSV_FLAG_NONE;

		m_pDevice->CreateDepthStencilView(m_pDepthBuffer.Get(), &viewDesc, handle);
		m_handleDSV = handle;
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
	// �X�V����
	{
		m_rotateAngle += 0.025f;
		m_CBV[m_frameIndex * 2 + 0].pBuffer->world = DirectX::XMMatrixRotationZ(m_rotateAngle + DirectX::XMConvertToRadians(45.0f));
		m_CBV[m_frameIndex * 2 + 1].pBuffer->world = DirectX::XMMatrixRotationY(m_rotateAngle) * DirectX::XMMatrixScaling(2.0f, 0.5f, 1.0f);
	}

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
	m_pCmdList->OMSetRenderTargets(1, &m_handleRTV[m_frameIndex], FALSE, &m_handleDSV);

	// �N���A�J���[�ݒ�
	float clearColor[] = {0.25f, 0.25f, 0.25f, 1.0f};

	// �����_�[�^�[�Q�b�g�r���[���N���A
	m_pCmdList->ClearRenderTargetView(m_handleRTV[m_frameIndex], clearColor, 0, nullptr);

	// �f�v�X�X�e���V���o�b�t�@�̃N���A
	m_pCmdList->ClearDepthStencilView(m_handleDSV, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// �`�揈��
	{
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
		m_pCmdList->SetDescriptorHeaps(1, m_pHeapCbvSrvUav.GetAddressOf());
		m_pCmdList->SetGraphicsRootDescriptorTable(1, m_texture.handleGPU);
		m_pCmdList->SetPipelineState(m_pPSO.Get());

		m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
		m_pCmdList->IASetIndexBuffer(&m_ibv);
		m_pCmdList->RSSetViewports(1, &m_viewport);
		m_pCmdList->RSSetScissorRects(1, &m_scissor);

		auto count = static_cast<uint32_t>(m_meshes[0].indices.size());
		
		m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_frameIndex * 2 + 0].desc.BufferLocation);
		m_pCmdList->DrawIndexedInstanced(count, 1, 0, 0, 0);

		//m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_frameIndex * 2 + 1].desc.BufferLocation);
		//m_pCmdList->DrawIndexedInstanced(count, 1, 0, 0, 0);
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

bool App::OnInit()
{
	// ���b�V�������[�h
	{
		std::wstring path = L"res/teapot/teapot.obj";
		if (!LoadMesh(path.c_str(), m_meshes, m_materials)) {
			return false;
		}

		assert(m_meshes.size() == 1);	// ���b�V����1����
	}

	// ���_�o�b�t�@�̐���
	{
		// ���_�f�[�^
		auto size = sizeof(MeshVertex) * m_meshes[0].vertices.size();
		auto vertices = m_meshes[0].vertices.data();

		// �q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// ���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pVB.ReleaseAndGetAddressOf())
		);

		if (FAILED(hr)) {
			return false;
		}

		// �}�b�s���O����
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr)) {
			return false;
		}

		// ���_�f�[�^���}�b�s���O��ɐݒ�
		memcpy(ptr, vertices, size);

		// �}�b�s���O����
		m_pVB->Unmap(0, nullptr);

		// ���_�o�b�t�@�r���[�̐ݒ�
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(size);
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(vertices));
	}

	// �C���f�N�X�o�b�t�@
	{
		auto size = sizeof(uint32_t) * m_meshes[0].indices.size();
		auto indices = m_meshes[0].indices.data();

		// �q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// ���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = size;
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		// ���\�[�X�𐶐�
		auto hr = m_pDevice->CreateCommittedResource(
			&prop,
			D3D12_HEAP_FLAG_NONE,
			&desc,
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(m_pIB.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			return false;
		}

		// �}�b�s���O
		void* ptr = nullptr;
		m_pIB->Map(0, nullptr, &ptr);
		if (FAILED(hr)) {
			return false;
		}

		// �C���f�b�N�X�f�[�^���R�s�[
		memcpy(ptr, indices, size);

		// �}�b�s���O����
		m_pIB->Unmap(0, nullptr);

		// �C���f�b�N�X�o�b�t�@�r���[�̐ݒ�
		m_ibv.BufferLocation = m_pIB->GetGPUVirtualAddress();
		m_ibv.Format = DXGI_FORMAT_R32_UINT;
		m_ibv.SizeInBytes = static_cast<UINT>(size);
	}

	// CBV/SRV/UAV�p�f�B�X�N���v�^�q�[�v�𐶐�
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 2 * FrameCount;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		auto hr = m_pDevice->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(m_pHeapCbvSrvUav.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			return false;
		}
	}

	// �萔�o�b�t�@�̐���
	{
		//�q�[�v�v���p�e�B
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// ���\�[�X�̐ݒ�
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		desc.Alignment = 0;
		desc.Width = sizeof(Transform);
		desc.Height = 1;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		desc.Flags = D3D12_RESOURCE_FLAG_NONE;

		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		for (auto i = 0; i < FrameCount * 2; i++)
		{
			auto hr = m_pDevice->CreateCommittedResource(
				&prop,
				D3D12_HEAP_FLAG_NONE,
				&desc,
				D3D12_RESOURCE_STATE_GENERIC_READ,
				nullptr,
				IID_PPV_ARGS(m_pCB[i].ReleaseAndGetAddressOf())
			);
			if (FAILED(hr)) {
				return false;
			}

			auto address = m_pCB[i]->GetGPUVirtualAddress();
			auto handleCPU = m_pHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart();
			auto handleGPU = m_pHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart();

			handleCPU.ptr += incrementSize * i;
			handleGPU.ptr += incrementSize * i;

			// �萔�o�b�t�@�r���[�̐ݒ�
			m_CBV[i].handleCPU = handleCPU;
			m_CBV[i].handleGPU = handleGPU;
			m_CBV[i].desc.BufferLocation = address;
			m_CBV[i].desc.SizeInBytes = sizeof(Transform);

			// �萔�o�b�t�@�r���[�̍쐬
			m_pDevice->CreateConstantBufferView(&m_CBV[i].desc, handleCPU);

			hr = m_pCB[i]->Map(0, nullptr, reinterpret_cast<void**>(&m_CBV[i].pBuffer));
			if (FAILED(hr)) {
				return false;
			}

			auto eyePos = DirectX::XMVectorSet(0.0f, 1.0f, 2.0f, 0.0f);
			auto targetPos = DirectX::XMVectorZero();
			auto upward = DirectX::XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);

			constexpr auto fovY = DirectX::XMConvertToRadians(37.5f);
			auto aspect = static_cast<float>(m_width) / static_cast<float>(m_height);

			// �ϊ��s��̐ݒ�
			m_CBV[i].pBuffer->world = DirectX::XMMatrixIdentity();
			m_CBV[i].pBuffer->view = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			m_CBV[i].pBuffer->proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
		}
	}

	// ���[�g�V�O�l�`������
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// ���[�g�p�����[�^�̐ݒ�
		D3D12_ROOT_PARAMETER param[2] = {};
		param[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param[0].Descriptor.ShaderRegister = 0;
		param[0].Descriptor.RegisterSpace = 0;
		param[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		D3D12_DESCRIPTOR_RANGE range = {};
		range.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		range.NumDescriptors = 1;
		range.BaseShaderRegister = 0;
		range.RegisterSpace = 0;
		range.OffsetInDescriptorsFromTableStart = 0;

		param[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		param[1].DescriptorTable.NumDescriptorRanges = 1;
		param[1].DescriptorTable.pDescriptorRanges = &range;
		param[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// �X�^�e�B�b�N�T���v���[�̐ݒ�
		D3D12_STATIC_SAMPLER_DESC sampler = {};
		sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
		sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
		sampler.MipLODBias = D3D12_DEFAULT_MIP_LOD_BIAS;
		sampler.MaxAnisotropy = 1;
		sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
		sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
		sampler.MinLOD = -D3D12_FLOAT32_MAX;
		sampler.MaxLOD = +D3D12_FLOAT32_MAX;
		sampler.ShaderRegister = 0;
		sampler.RegisterSpace = 0;
		sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		// ���[�g�V�O�l�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 2;
		desc.NumStaticSamplers = 1;
		desc.pParameters = param;
		desc.pStaticSamplers = &sampler;
		desc.Flags = flag;

		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// �V���A���C�Y
		auto hr = D3D12SerializeRootSignature(
			&desc,
			D3D_ROOT_SIGNATURE_VERSION_1_0,
			pBlob.GetAddressOf(),
			pErrorBlob.GetAddressOf()
		);
		if (FAILED(hr)) {
			return false;
		}

		hr = m_pDevice->CreateRootSignature(
			0,
			pBlob->GetBufferPointer(),
			pBlob->GetBufferSize(),
			IID_PPV_ARGS(m_pRootSignature.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			return false;
		}
	}

	// �p�C�v���C���X�e�[�g�̐���
	{
		// ���X�^���C�U�[�X�e�[�g�̐ݒ�
		D3D12_RASTERIZER_DESC descRS = {};
		descRS.FillMode = D3D12_FILL_MODE_SOLID;
		descRS.CullMode = D3D12_CULL_MODE_NONE;
		descRS.FrontCounterClockwise = FALSE;
		descRS.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
		descRS.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
		descRS.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
		descRS.DepthClipEnable = FALSE;
		descRS.MultisampleEnable = FALSE;
		descRS.AntialiasedLineEnable = FALSE;
		descRS.ForcedSampleCount = 0;
		descRS.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

		// �����_�[�^�[�Q�b�g�̃u�����h�ݒ�
		D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		// �u�����h�X�e�[�g�̐ݒ�
		D3D12_BLEND_DESC descBS;
		descBS.AlphaToCoverageEnable = FALSE;
		descBS.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			descBS.RenderTarget[i] = descRTBS;
		}

		// �f�v�X�X�e���V���X�e�[�g�̐ݒ�
		D3D12_DEPTH_STENCIL_DESC descDSS = {};
		descDSS.DepthEnable = TRUE;
		descDSS.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		descDSS.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		descDSS.StencilEnable = FALSE;

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// ���_�V�F�[�_�ǂݍ���
		auto hr = D3DReadFileToBlob(L"SimpleTexVS.cso", pVSBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// �s�N�Z���V�F�[�_�ǂݍ���
		hr = D3DReadFileToBlob(L"SimpleTexPS.cso", pPSBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// �p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = MeshVertex::InputLayout;
		desc.pRootSignature = m_pRootSignature.Get();
		desc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		desc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		desc.RasterizerState = descRS;
		desc.BlendState = descBS;
		desc.DepthStencilState = descDSS;;
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;

		hr = m_pDevice->CreateGraphicsPipelineState(
			&desc,
			IID_PPV_ARGS(m_pPSO.ReleaseAndGetAddressOf())
		);
		if (FAILED(hr)) {
			return false;
		}
	}

	// �e�N�X�`������
	{
		// �t�@�C���p�X
		std::wstring texturePath = TEXT("res/teapot/default.DDS");

		DirectX::ResourceUploadBatch batch(m_pDevice.Get());
		batch.Begin();

		// ���\�[�X�𐶐�
		auto hr = DirectX::CreateDDSTextureFromFile(
			m_pDevice.Get(),
			batch,
			texturePath.c_str(),
			m_texture.pResource.ReleaseAndGetAddressOf(),
			true
		);
		if (FAILED(hr)) {
			return false;
		}

		// �R�}���h�����s
		auto future = batch.End(m_pQueue.Get());

		// �R�}���h�̊�����ҋ@
		future.wait();

		// �C���N�������g�T�C�Y���擾
		auto incrementSize = m_pDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		// CPU�f�B�X�N���v�^�n���h����GPU�f�B�X�N���v�^�n���h�����f�B�X�N���v�^�q�[�v����擾
		auto handleCPU = m_pHeapCbvSrvUav->GetCPUDescriptorHandleForHeapStart();
		auto handleGPU = m_pHeapCbvSrvUav->GetGPUDescriptorHandleForHeapStart();

		// �e�N�X�`���Ƀf�B�X�N���v�^�����蓖�Ă�
		handleCPU.ptr += incrementSize * 2;
		handleGPU.ptr += incrementSize * 2;

		m_texture.handleCPU = handleCPU;
		m_texture.handleGPU = handleGPU;

		// �e�N�X�`���̍\���ݒ���擾
		auto textureDesc = m_texture.pResource->GetDesc();

		// �V�F�[�_���\�[�X�r���[�̐ݒ�
		D3D12_SHADER_RESOURCE_VIEW_DESC viewDesc = {};
		viewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		viewDesc.Format = textureDesc.Format;
		viewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		viewDesc.Texture2D.MostDetailedMip = 0;
		viewDesc.Texture2D.MipLevels = textureDesc.MipLevels;
		viewDesc.Texture2D.PlaneSlice = 0;
		viewDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		// �V�F�[�_�[���\�[�X�r���[�𐶐�
		m_pDevice->CreateShaderResourceView(m_texture.pResource.Get(), &viewDesc, handleCPU);
	}

	// �r���[�|�[�g�ƃV�U�[�̐ݒ�
	{
		m_viewport.TopLeftX = 0;
		m_viewport.TopLeftY = 0;
		m_viewport.Width = static_cast<float>(m_width);
		m_viewport.Height = static_cast<float>(m_height);
		m_viewport.MinDepth = 0.0f;
		m_viewport.MaxDepth = 1.0f;

		m_scissor.left = 0;
		m_scissor.right = m_width;
		m_scissor.top = 0;
		m_scissor.bottom = m_height;
	}

	return true;
}

void App::OnTerm()
{
	for (auto i = 0u; i < FrameCount; i++)
	{
		if (m_pCB[i].Get() != nullptr) {
			m_pCB[i]->Unmap(0, nullptr);
			memset(&m_CBV[i], 0, sizeof(m_CBV[i]));
		}
		m_pCB[i].Reset();
	}

	m_pVB.Reset();
	m_pPSO.Reset();
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
