#pragma once

#include <Windows.h>
#include <cstdint>
#include <array>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>

#include <ComPtr.h>
#include <DescriptorPool.h>
#include <ColorTarget.h>
#include <DepthTarget.h>
#include <CommandList.h>
#include <Fence.h>
#include <Mesh.h>
#include <Texture.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")


class App
{
public:
	App(uint32_t width, uint32_t height);
	~App();

	void Run();

protected:
	enum POOL_TYPE {
		POOL_TYPE_RES,	// CBV/SRV/UAV
		POOL_TYPE_SMP,	// Sampler
		POOL_TYPE_RTV,	// RTV
		POOL_TYPE_DSV,	// DSV
		POOL_COUNT,
	};

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
	static constexpr uint32_t FrameCount = 2;	//!< �t���[���o�b�t�@��

	HINSTANCE m_instance;	//!< �C���X�^���X�n���h��
	HWND m_hwnd;			//!< �E�C���h�E�n���h��
	uint32_t m_width;		//!< �E�C���h�E�̕�
	uint32_t m_height;		//!< �E�C���h�E�̍���

	ComPtr<ID3D12Device> m_pDevice;	//!< �f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue;	//!< �R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain;	//!< �X���b�v�`�F�C��
	std::array<ColorTarget, FrameCount> m_colorTargets = {};	//!< �J���[�^�[�Q�b�g
	DepthTarget m_depthTarget;	//!< �[�x�^�[�Q�b�g
	std::array<DescriptorPool*, POOL_COUNT> m_pPools;	//!< �f�B�X�N���v�^�v�[��
	CommandList m_commandList;	//!< �R�}���h���X�g
	Fence m_fence;	//!< �t�F���X
	uint32_t m_frameIndex;	//!< �t���[���C���f�b�N�X
	D3D12_VIEWPORT					m_viewport;	// �r���[�|�[�g
	D3D12_RECT						m_scissor;	// �V�U�[��`
};

