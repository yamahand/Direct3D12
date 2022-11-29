#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

class App
{
public:
	App(uint32_t width, uint32_t height);
	~App();

	void Run();

private:
	bool InitApp();
	void TermApp();

	bool InitWnd();
	void TermWnd();

	bool InitD3D();
	void TermD3D();

	void Render();
	void WaitGpu();
	void Present(uint32_t interval);

	void MainLoop();

	static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp);

private:
	static const uint32_t FrameCount = 2;	//!< フレームバッファ数

	HINSTANCE m_instance;	//!< インスタンスハンドル
	HWND m_hwnd;			//!< ウインドウハンドル
	uint32_t m_width;		//!< ウインドウの幅
	uint32_t m_height;		//!< ウインドウの高さ

	ComPtr<ID3D12Device> m_pDevice;	//!< デバイス
	ComPtr<ID3D12CommandQueue> m_pQueue;	//!< コマンドキュー
	ComPtr<IDXGISwapChain3> m_pSwapChain;	//!< スワップチェイン
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];	//!< カラーバッファ
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];	//!< コマンドアロケータ
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;	//!< コマンドリスト
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;	//!< レンダーターゲットディスクリプタヒープ
	ComPtr<ID3D12Fence> m_pFence;	//!< フェンス
	HANDLE m_fenceEvent = {};			//!< フェンスイベント
	uint64_t m_fenceCounter[FrameCount] = {};	//!< フェンスカウンター
	uint32_t m_frameIndex = 0;					//!< フレームインデックス
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleRTV[FrameCount] = {};	//!< レンダーターゲット用CPUディスクリプタ

};

