#include "App.h"
#include "ResourceUploadBatch.h"
#include "DDSTextureLoader.h"
#include "VertexTypes.h"
#include "ResMesh.h"
#include <cassert>
#include "DescriptorPool.h"

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

	// ウインドウ表示
	ShowWindow(m_hwnd, SW_SHOWNORMAL);

	// ウインドウを更新
	UpdateWindow(m_hwnd);

	// ウインドウにフォーカスを設定
	SetFocus(m_hwnd);

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
		this
	);

	if (m_hwnd == nullptr) {
		return false;
	}

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

		hr = m_pDevice->CreateCommandQueue(&desc, IID_PPV_ARGS(m_pQueue.GetAddressOf()));
		if (FAILED(hr))
			return false;
	}

	// スワップチェインの生成
	{
		// DXGIファクトリーの生成
		ComPtr<IDXGIFactory4> pFactory = nullptr;
		hr = CreateDXGIFactory1(IID_PPV_ARGS(pFactory.GetAddressOf()));
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
		ComPtr<IDXGISwapChain> pSwapChain;
		hr = pFactory->CreateSwapChain(m_pQueue.Get(), &desc, pSwapChain.GetAddressOf());
		if (FAILED(hr)) {
			return false;
		}

		// IDXGISwapChain3を取得
		hr = pSwapChain.As(&m_pSwapChain);
		if (FAILED(hr)) {
			return false;
		}

		// バックバッファインデックスを取得
		m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();

		pFactory.Reset();
		pSwapChain.Reset();
	}

	// ディスクリプタプール作成
	{
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};

		desc.NodeMask = 1;
		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		desc.NumDescriptors = 512;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (!DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPools[POOL_TYPE_RES])) {
			return false;
		}

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
		desc.NumDescriptors = 256;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (!DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPools[POOL_TYPE_SMP])) {
			return false;
		}

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		desc.NumDescriptors = 512;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (!DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPools[POOL_TYPE_RTV])) {
			return false;
		}

		desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		desc.NumDescriptors = 512;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		if (!DescriptorPool::Create(m_pDevice.Get(), &desc, &m_pPools[POOL_TYPE_DSV])) {
			return false;
		}
	}

	// コマンドリストの生成
	{
		if (!m_commandList.Init(m_pDevice.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT, FrameCount)) {
			return false;
		}
	}

	// レンダーターゲットビューの生成
	{
		for (auto i = 0u; i < FrameCount; i++)
		{
			if (!m_colorTargets[i].InitFromBackBuffer(m_pDevice.Get(), m_pPools[POOL_TYPE_RTV], true, i, m_pSwapChain.Get())) {
				return false;
			}
		}
	}

	// デプスステンシルバッファの作成
	{
		if (!m_depthTarget.Init(
			m_pDevice.Get(),
			m_pPools[POOL_TYPE_DSV],
			m_width,
			m_height,
			DXGI_FORMAT_D32_FLOAT)) {
			return false;
		}
	}

	// フェンスの生成
	{
		if (!m_fence.Init(
			m_pDevice.Get()
		)) {
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

void App::TermD3D()
{
	// GPU処理の完了を待機
	m_fence.Sync(m_pQueue.Get());

	// フェンス破棄
	m_fence.Term();

	// レンダーターゲットビューの破棄
	for (auto i = 0u; i < FrameCount; ++i)
	{
		m_colorTargets[i].Term();
	}

	// 深度ステンシルビューの破棄
	m_depthTarget.Term();

	// コマンドリストの破棄
	m_commandList.Term();

	for (auto i = 0; i < POOL_COUNT; i++)
	{
		if (m_pPools[i] != nullptr) {
			m_pPools[i]->Release();
			m_pPools[i] = nullptr;
		}
	}

	// スワップチェインの破棄
	m_pSwapChain.Reset();

	// コマンドキューの破棄
	m_pQueue.Reset();

	// デバイスは破棄
	m_pDevice.Reset();

}

void App::Present(uint32_t interval)
{
	// 画面に表示
	m_pSwapChain->Present(interval, 0);

	// 完了待ち
	m_fence.Wait(m_pQueue.Get(), INFINITE);

	// フレーム番号を更新
	m_frameIndex = m_pSwapChain->GetCurrentBackBufferIndex();
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
		else {
			OnRender();
		}
	}
}

LRESULT CALLBACK App::WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
	auto instance = reinterpret_cast<App*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));

	switch (msg)
	{
	case WM_CREATE:
	{
		auto pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lp);
		SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	if (instance != nullptr) {
		instance->OnMsgProc(hwnd, msg, wp, lp);
	}

	return DefWindowProc(hwnd, msg, wp, lp);
}
