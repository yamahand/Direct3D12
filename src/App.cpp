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

	struct Vertex {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
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
		m_rotateAngle += 0.25f;
		m_CBV[m_frameIndex].pBuffer->world = DirectX::XMMatrixRotationY(m_rotateAngle);
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
	m_pCmdList->OMSetRenderTargets(1, &m_handleRTV[m_frameIndex], FALSE, nullptr);

	// �N���A�J���[�ݒ�
	float clearColor[] = {0.25f, 0.25f, 0.25f, 1.0f};

	// �����_�[�^�[�Q�b�g�r���[��kuria 
	m_pCmdList->ClearRenderTargetView(m_handleRTV[m_frameIndex], clearColor, 0, nullptr);

	// �`�揈��
	{
		m_pCmdList->SetGraphicsRootSignature(m_pRootSignature.Get());
		m_pCmdList->SetDescriptorHeaps(1, m_pHeapCBV.GetAddressOf());
		m_pCmdList->SetGraphicsRootConstantBufferView(0, m_CBV[m_frameIndex].desc.BufferLocation);
		m_pCmdList->SetPipelineState(m_pPSO.Get());

		m_pCmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_pCmdList->IASetVertexBuffers(0, 1, &m_VBV);
		m_pCmdList->RSSetViewports(1, &m_viewport);
		m_pCmdList->RSSetScissorRects(1, &m_scissor);

		m_pCmdList->DrawInstanced(3, 1, 0, 0);
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
	// ���_�o�b�t�@�̐���
	{
		// ���_�f�[�^
		Vertex vertices[] = {
			{ DirectX::XMFLOAT3( -1.0f, -1.0f,  0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
			{ DirectX::XMFLOAT3(  1.0f, -1.0f,  0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		};

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
		desc.Width = sizeof(vertices);
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
		memcpy(ptr, vertices, sizeof(vertices));

		// �}�b�s���O����
		m_pVB->Unmap(0, nullptr);

		// ���_�o�b�t�@�r���[�̐ݒ�
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
	}

	// �萔�o�b�t�@�p�f�B�X�N���v�^�q�[�v�𐶐�
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 1 * FrameCount;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		desc.NodeMask = 0;

		auto hr = m_pDevice->CreateDescriptorHeap(
			&desc,
			IID_PPV_ARGS(m_pHeapCBV.ReleaseAndGetAddressOf())
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

		for (auto i = 0; i < FrameCount; i++)
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
			auto handleCPU = m_pHeapCBV->GetCPUDescriptorHandleForHeapStart();
			auto handleGPU = m_pHeapCBV->GetGPUDescriptorHandleForHeapStart();

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

			auto eyePos = DirectX::XMVectorSet(0.0f, 0.0f, 5.0f, 0.0f);
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
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.Descriptor.ShaderRegister = 0;
		param.Descriptor.RegisterSpace = 0;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// ���[�g�V�O�l�`���̐ݒ�
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 1;
		desc.NumStaticSamplers = 0;
		desc.pParameters = &param;
		desc.pStaticSamplers = nullptr;
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
		// ���̓��C�A�E�g�̐ݒ�
		D3D12_INPUT_ELEMENT_DESC elements[2];
		elements[0].SemanticName = "POSITION";
		elements[0].SemanticIndex = 0;
		elements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		elements[0].InputSlot = 0;
		elements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		elements[0].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elements[0].InstanceDataStepRate = 0;

		elements[1].SemanticName = "COLOR";
		elements[1].SemanticIndex = 0;
		elements[1].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
		elements[1].InputSlot = 0;
		elements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		elements[1].InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
		elements[1].InstanceDataStepRate = 0;

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

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// ���_�V�F�[�_�ǂݍ���
		auto hr = D3DReadFileToBlob(L"SimpleVS.cso", pVSBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// �s�N�Z���V�F�[�_�ǂݍ���
		hr = D3DReadFileToBlob(L"SimplePS.cso", pPSBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// �p�C�v���C���X�e�[�g�̐ݒ�
		D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
		desc.InputLayout = { elements, _countof(elements) };
		desc.pRootSignature = m_pRootSignature.Get();
		desc.VS = { pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize() };
		desc.PS = { pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize() };
		desc.RasterizerState = descRS;
		desc.BlendState = descBS;
		desc.DepthStencilState.DepthEnable = FALSE;
		desc.DepthStencilState.StencilEnable = FALSE;
		desc.SampleMask = UINT_MAX;
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.NumRenderTargets = 1;
		desc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
		desc.DSVFormat = DXGI_FORMAT_UNKNOWN;
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
