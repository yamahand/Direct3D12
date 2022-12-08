#pragma once

#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

template<typename T> using ComPtr = Microsoft::WRL::ComPtr<T>;

template<typename T>
struct ConstantBufferView {
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;		// 定数バッファ構成設定
	D3D12_CPU_DESCRIPTOR_HANDLE		handleCPU;	// CPUディスクリプタハンドル
	D3D12_GPU_DESCRIPTOR_HANDLE		handleGPU;	// GPUディスクリプタハンドル
	T* pBuffer;		// バッファの先頭へのポインタ
};

struct alignas(256) Transform {
	DirectX::XMMATRIX	world;
	DirectX::XMMATRIX	view;
	DirectX::XMMATRIX	proj;
};

struct Texture {
	ComPtr<ID3D12Resource>	pResource;
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;
};

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

	bool OnInit();
	void OnTerm();

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
	ComPtr<ID3D12Resource> m_pDepthBuffer;	//!< デプスバッファ
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];	//!< コマンドアロケータ
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;	//!< コマンドリスト
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;	//!< レンダーターゲットディスクリプタヒープ
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV;	//!< デプスステンシルディスクリプタヒープ
	ComPtr<ID3D12Fence> m_pFence;	//!< フェンス
	ComPtr<ID3D12DescriptorHeap>	m_pHeapCbvSrvUav;	// ディスクリプタヒープ(定数バッファ・シェーダリソースビュー・アンオーダードアクセスビュー)
	ComPtr<ID3D12Resource>			m_pVB;		// 頂点バッファ
	ComPtr<ID3D12Resource>			m_pIB;		// インデックスバッファ
	ComPtr<ID3D12Resource>			m_pCB[FrameCount * 2];	// 定数バッファ
	ComPtr<ID3D12RootSignature>		m_pRootSignature;	// ルートシグネチャ
	ComPtr<ID3D12PipelineState>		m_pPSO;				// パイプラインステートオブジェクト

	HANDLE m_fenceEvent = {};			//!< フェンスイベント
	uint64_t m_fenceCounter[FrameCount] = {};	//!< フェンスカウンター
	uint32_t m_frameIndex = 0;					//!< フレームインデックス
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleRTV[FrameCount] = {};	//!< レンダーターゲット用CPUディスクリプタ
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleDSV = {};	//!< デプスステンシル用CPUディスクリプタ

	Texture							m_texture;

	D3D12_VERTEX_BUFFER_VIEW		m_VBV;	// 頂点バッファビュー
	D3D12_INDEX_BUFFER_VIEW			m_ibv;	// インデックスバッファビュー
	D3D12_VIEWPORT					m_viewport;	// ビューポート
	D3D12_RECT						m_scissor;	// シザー矩形
	ConstantBufferView<Transform>	m_CBV[FrameCount * 2];
	float							m_rotateAngle;	// 回転角
};

