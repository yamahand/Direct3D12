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
/// コンストラクタ
/// </summary>
/// <param name="width">ウインドウの幅</param>
/// <param name="height">ウインドウの高さ</param>
App::App(uint32_t width, uint32_t height)
	: m_instance(nullptr)
	, m_hwnd(nullptr)
	, m_width(width)
	, m_height(height)
{
}

/// <summary>
/// デストラクタ
/// </summary>
App::~App()
{

}

/// <summary>
/// 実行
/// </summary>
void App::Run() {
	if (InitApp()) {
		MainLoop();
	}

	TermApp();
}

/// <summary>
/// 初期化処理
/// </summary>
/// <returns>初期化に成功したか</returns>
bool App::InitApp() {
#if defined(DEBUG) || defined(_DEBUG)
	{
		ComPtr<ID3D12Debug> debug;
		auto hr = D3D12GetDebugInterface(IID_PPV_ARGS(debug.GetAddressOf()));

		// デバッグレイヤーを有効化
		if (SUCCEEDED(hr)) {
			debug->EnableDebugLayer();
		}
	}
#endif

	// ウインドウの初期化
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
/// 終了処理
/// </summary>
void App::TermApp() {
	OnTerm();
	TermD3D();
	TermWnd();
}

/// <summary>
/// ウインドウの初期化処理
/// </summary>
/// <returns></returns>
bool App::InitWnd() {
	auto hInstance = GetModuleHandle(nullptr);
	if (hInstance == nullptr)
		return false;

	// ウインドウ設定
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

	// ウインドウ登録
	if (!RegisterClassEx(&wc)) {
		return false;
	}

	// インスタンスハンドル設定
	m_instance = hInstance;

	// ウインドウのサイズを設定
	RECT rc = {};
	rc.right = static_cast<LONG>(m_width);
	rc.bottom = static_cast<LONG>(m_height);

	// ウインドウサイズを調整
	auto style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU;
	AdjustWindowRect(&rc, style, FALSE);

	// ウインドウを生成
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

	// ウインドウ表示
	ShowWindow(m_hwnd, SW_SHOWNORMAL);

	// ウインドウを更新
	UpdateWindow(m_hwnd);

	// ウインドウにフォーカスを設定
	SetFocus(m_hwnd);

	return true;
}

/// <summary>
/// ウインドウの終了処理
/// </summary>
void App::TermWnd() {
	// ウインドウの登録を解除
	if (m_instance != nullptr) {
		UnregisterClass(ClassName, m_instance);
	}

	m_instance = nullptr;
	m_hwnd = nullptr;
}

bool App::InitD3D()
{
	// デバイスの生成
	auto hr = D3D12CreateDevice(
		nullptr,
		D3D_FEATURE_LEVEL_12_0,
		IID_PPV_ARGS(&m_pDevice)
	);

	if (FAILED(hr))
		return false;

	// コマンドキューの生成
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

	// スワップチェインの生成
	{
		// DXGIファクトリーの生成
		IDXGIFactory4* pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
		if (FAILED(hr))
			return false;

		// スワップチェインの設定
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

		// スワップチェインの生成
		IDXGISwapChain* pSwapChain = nullptr;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, &pSwapChain);
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			return false;
		}

		// IDXGISwapChain3を取得
		hr = pSwapChain->QueryInterface(IID_PPV_ARGS(&m_pSwapChain));
		if (FAILED(hr)) {
			SafeRelease(pFactory);
			SafeRelease(pSwapChain);
			return false;
		}

		// バックバッファインデックスを取得
		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		SafeRelease(pFactory);
		SafeRelease(pSwapChain);
	}

	// コマンドアロケータの生成
	{
		for (auto i = 0u; i < FrameCount; ++i)
		{
			hr = m_pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_pCmdAllocator[i]));
			if (FAILED(hr)) {
				return false;
			}
		}
	}

	// コマンドリストの生成
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

	// レンダーターゲットビュー
	{
		// ディスクリプタヒープの設定
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		desc.NumDescriptors = FrameCount;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NodeMask = 0;

		// ディスクリプタヒープを生成
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

			// レンダーターゲットビューの生成
			m_pDevice->CreateRenderTargetView(m_pColorBuffer[i].Get(), &viewDesc, handle);

			m_handleRTV[i] = handle;
			handle.ptr += incrementSize;
		}
	}

	// フェンスの生成
	{
		// フェンスカウンターをリセット
		for (auto i = 0; i < FrameCount; ++i)
		{
			m_fenceCounter[i] = 0;
		}
		// フェンスの生成
		hr = m_pDevice->CreateFence(
			m_fenceCounter[m_frameIndex],
			D3D12_FENCE_FLAG_NONE,
			IID_PPV_ARGS(&m_pFence)
		);
		if (FAILED(hr)) {
			return false;
		}

		++m_fenceCounter[m_frameIndex];

		// イベントの生成
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr) {
			return false;
		}
	}

	// コマンドを閉じる
	m_pCmdList->Close();

	return true;
}

void App::TermD3D()
{
	// GPU処理の完了を待機
	WaitGpu();

	// イベント破棄
	if (m_fenceEvent != nullptr) {
		CloseHandle(m_fenceEvent);
		m_fenceEvent = nullptr;
	}

	// フェンス破棄
	m_pFence.Reset();

	// レンダーターゲットビューの破棄
	m_pHeapRTV.Reset();
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pColorBuffer[i].Reset();
	}

	// コマンドリストの破棄
	m_pCmdList.Reset();

	// コマンドアロケータの破棄
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_pCmdAllocator[i].Reset();
	}

	// スワップチェインの破棄
	m_pSwapChain.Reset();

	// コマンドキューの破棄
	m_pQueue.Reset();

	// デバイスは破棄
	m_pDevice.Reset();

}

void App::Render()
{
	// 更新処理
	{
		m_rotateAngle += 0.25f;
		m_CBV[m_frameIndex].pBuffer->world = DirectX::XMMatrixRotationY(m_rotateAngle);
	}

	// コマンドの記録開始
	m_pCmdAllocator[m_frameIndex]->Reset();
	m_pCmdList->Reset(m_pCmdAllocator[m_frameIndex].Get(), nullptr);

	// リソースバリアの設定
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// リソースバリア
	m_pCmdList->ResourceBarrier(1, &barrier);

	// レンダーターゲットの設定
	m_pCmdList->OMSetRenderTargets(1, &m_handleRTV[m_frameIndex], FALSE, nullptr);

	// クリアカラー設定
	float clearColor[] = {0.25f, 0.25f, 0.25f, 1.0f};

	// レンダーターゲットビューをkuria 
	m_pCmdList->ClearRenderTargetView(m_handleRTV[m_frameIndex], clearColor, 0, nullptr);

	// 描画処理
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

	// リソースバリア設定
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = m_pColorBuffer[m_frameIndex].Get();
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	// リソースバリア
	m_pCmdList->ResourceBarrier(1, &barrier);

	// コマンド記録を終了
	m_pCmdList->Close();

	// コマンドを実行
	ID3D12CommandList* ppCmdLists[] = { m_pCmdList.Get()};
	m_pQueue->ExecuteCommandLists(1, ppCmdLists);

	// 画面に表示
	Present(1);
}

void App::WaitGpu()
{
	// シグナル処理
	m_pQueue->Signal(m_pFence.Get(), m_fenceCounter[m_frameIndex]);

	// 完了時にイベントを設定する
	m_pFence->SetEventOnCompletion(m_fenceCounter[m_frameIndex], m_fenceEvent);

	// 待機処理
	WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);

	++m_fenceCounter[m_frameIndex];
}

void App::Present(uint32_t interval)
{
	// 画面に表示
	m_pSwapChain->Present(interval, 0);

	// シグナル処理
	const auto currentValue = m_fenceCounter[m_frameIndex];
	m_pQueue->Signal(m_pFence.Get(), currentValue);

	// バックバッファ番号を更新
	m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

	// 次のフレームの描画準備ができていないなら待機する
	if (m_pFence->GetCompletedValue() < m_fenceCounter[m_frameIndex]) {
		m_pFence->SetEventOnCompletion(m_fenceCounter[m_frameIndex], m_fenceEvent);
		WaitForSingleObjectEx(m_fenceEvent, INFINITE, FALSE);
	}

	// 次のフレームのフェンスカウンターを増やす
	m_fenceCounter[m_frameIndex] = currentValue + 1;
}

bool App::OnInit()
{
	// 頂点バッファの生成
	{
		// 頂点データ
		Vertex vertices[] = {
			{ DirectX::XMFLOAT3( -1.0f, -1.0f,  0.0f), DirectX::XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) },
			{ DirectX::XMFLOAT3(  1.0f, -1.0f,  0.0f), DirectX::XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) },
			{ DirectX::XMFLOAT3(  0.0f,  1.0f,  0.0f), DirectX::XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) },
		};

		// ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// リソースの設定
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

		// マッピングする
		void* ptr = nullptr;
		hr = m_pVB->Map(0, nullptr, &ptr);
		if (FAILED(hr)) {
			return false;
		}

		// 頂点データをマッピング先に設定
		memcpy(ptr, vertices, sizeof(vertices));

		// マッピング解除
		m_pVB->Unmap(0, nullptr);

		// 頂点バッファビューの設定
		m_VBV.BufferLocation = m_pVB->GetGPUVirtualAddress();
		m_VBV.SizeInBytes = static_cast<UINT>(sizeof(vertices));
		m_VBV.StrideInBytes = static_cast<UINT>(sizeof(Vertex));
	}

	// 定数バッファ用ディスクリプタヒープを生成
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

	// 定数バッファの生成
	{
		//ヒーププロパティ
		D3D12_HEAP_PROPERTIES prop = {};
		prop.Type = D3D12_HEAP_TYPE_UPLOAD;
		prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		prop.CreationNodeMask = 1;
		prop.VisibleNodeMask = 1;

		// リソースの設定
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

			// 定数バッファビューの設定
			m_CBV[i].handleCPU = handleCPU;
			m_CBV[i].handleGPU = handleGPU;
			m_CBV[i].desc.BufferLocation = address;
			m_CBV[i].desc.SizeInBytes = sizeof(Transform);

			// 定数バッファビューの作成
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

			// 変換行列の設定
			m_CBV[i].pBuffer->world = DirectX::XMMatrixIdentity();
			m_CBV[i].pBuffer->view = DirectX::XMMatrixLookAtRH(eyePos, targetPos, upward);
			m_CBV[i].pBuffer->proj = DirectX::XMMatrixPerspectiveFovRH(fovY, aspect, 1.0f, 1000.0f);
		}
	}

	// ルートシグネチャ生成
	{
		auto flag = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;
		flag |= D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;

		// ルートパラメータの設定
		D3D12_ROOT_PARAMETER param = {};
		param.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		param.Descriptor.ShaderRegister = 0;
		param.Descriptor.RegisterSpace = 0;
		param.ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

		// ルートシグネチャの設定
		D3D12_ROOT_SIGNATURE_DESC desc = {};
		desc.NumParameters = 1;
		desc.NumStaticSamplers = 0;
		desc.pParameters = &param;
		desc.pStaticSamplers = nullptr;
		desc.Flags = flag;

		ComPtr<ID3DBlob> pBlob;
		ComPtr<ID3DBlob> pErrorBlob;

		// シリアライズ
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

	// パイプラインステートの生成
	{
		// 入力レイアウトの設定
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

		// ラスタライザーステートの設定
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

		// レンダーターゲットのブレンド設定
		D3D12_RENDER_TARGET_BLEND_DESC descRTBS = {
			FALSE, FALSE,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP,
			D3D12_COLOR_WRITE_ENABLE_ALL
		};

		// ブレンドステートの設定
		D3D12_BLEND_DESC descBS;
		descBS.AlphaToCoverageEnable = FALSE;
		descBS.IndependentBlendEnable = FALSE;
		for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		{
			descBS.RenderTarget[i] = descRTBS;
		}

		ComPtr<ID3DBlob> pVSBlob;
		ComPtr<ID3DBlob> pPSBlob;

		// 頂点シェーダ読み込み
		auto hr = D3DReadFileToBlob(L"SimpleVS.cso", pVSBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// ピクセルシェーダ読み込み
		hr = D3DReadFileToBlob(L"SimplePS.cso", pPSBlob.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// パイプラインステートの設定
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

	// ビューポートとシザーの設定
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
/// メインループ
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
