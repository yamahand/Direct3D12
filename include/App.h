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
	static const uint32_t FrameCount = 2;	//!< �t���[���o�b�t�@��

	HINSTANCE m_instance;	//!< �C���X�^���X�n���h��
	HWND m_hwnd;			//!< �E�C���h�E�n���h��
	uint32_t m_width;		//!< �E�C���h�E�̕�
	uint32_t m_height;		//!< �E�C���h�E�̍���

	ComPtr<ID3D12Device> m_pDevice;	//!< �f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue;	//!< �R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain;	//!< �X���b�v�`�F�C��
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];	//!< �J���[�o�b�t�@
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];	//!< �R�}���h�A���P�[�^
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;	//!< �R�}���h���X�g
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;	//!< �����_�[�^�[�Q�b�g�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Fence> m_pFence;	//!< �t�F���X
	HANDLE m_fenceEvent = {};			//!< �t�F���X�C�x���g
	uint64_t m_fenceCounter[FrameCount] = {};	//!< �t�F���X�J�E���^�[
	uint32_t m_frameIndex = 0;					//!< �t���[���C���f�b�N�X
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleRTV[FrameCount] = {};	//!< �����_�[�^�[�Q�b�g�pCPU�f�B�X�N���v�^

};

