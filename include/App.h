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
	D3D12_CONSTANT_BUFFER_VIEW_DESC desc;		// �萔�o�b�t�@�\���ݒ�
	D3D12_CPU_DESCRIPTOR_HANDLE		handleCPU;	// CPU�f�B�X�N���v�^�n���h��
	D3D12_GPU_DESCRIPTOR_HANDLE		handleGPU;	// GPU�f�B�X�N���v�^�n���h��
	T* pBuffer;		// �o�b�t�@�̐擪�ւ̃|�C���^
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
	static const uint32_t FrameCount = 2;	//!< �t���[���o�b�t�@��

	HINSTANCE m_instance;	//!< �C���X�^���X�n���h��
	HWND m_hwnd;			//!< �E�C���h�E�n���h��
	uint32_t m_width;		//!< �E�C���h�E�̕�
	uint32_t m_height;		//!< �E�C���h�E�̍���

	ComPtr<ID3D12Device> m_pDevice;	//!< �f�o�C�X
	ComPtr<ID3D12CommandQueue> m_pQueue;	//!< �R�}���h�L���[
	ComPtr<IDXGISwapChain3> m_pSwapChain;	//!< �X���b�v�`�F�C��
	ComPtr<ID3D12Resource> m_pColorBuffer[FrameCount];	//!< �J���[�o�b�t�@
	ComPtr<ID3D12Resource> m_pDepthBuffer;	//!< �f�v�X�o�b�t�@
	ComPtr<ID3D12CommandAllocator> m_pCmdAllocator[FrameCount];	//!< �R�}���h�A���P�[�^
	ComPtr<ID3D12GraphicsCommandList> m_pCmdList;	//!< �R�}���h���X�g
	ComPtr<ID3D12DescriptorHeap> m_pHeapRTV;	//!< �����_�[�^�[�Q�b�g�f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12DescriptorHeap> m_pHeapDSV;	//!< �f�v�X�X�e���V���f�B�X�N���v�^�q�[�v
	ComPtr<ID3D12Fence> m_pFence;	//!< �t�F���X
	ComPtr<ID3D12DescriptorHeap>	m_pHeapCbvSrvUav;	// �f�B�X�N���v�^�q�[�v(�萔�o�b�t�@�E�V�F�[�_���\�[�X�r���[�E�A���I�[�_�[�h�A�N�Z�X�r���[)
	ComPtr<ID3D12Resource>			m_pVB;		// ���_�o�b�t�@
	ComPtr<ID3D12Resource>			m_pIB;		// �C���f�b�N�X�o�b�t�@
	ComPtr<ID3D12Resource>			m_pCB[FrameCount * 2];	// �萔�o�b�t�@
	ComPtr<ID3D12RootSignature>		m_pRootSignature;	// ���[�g�V�O�l�`��
	ComPtr<ID3D12PipelineState>		m_pPSO;				// �p�C�v���C���X�e�[�g�I�u�W�F�N�g

	HANDLE m_fenceEvent = {};			//!< �t�F���X�C�x���g
	uint64_t m_fenceCounter[FrameCount] = {};	//!< �t�F���X�J�E���^�[
	uint32_t m_frameIndex = 0;					//!< �t���[���C���f�b�N�X
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleRTV[FrameCount] = {};	//!< �����_�[�^�[�Q�b�g�pCPU�f�B�X�N���v�^
	D3D12_CPU_DESCRIPTOR_HANDLE m_handleDSV = {};	//!< �f�v�X�X�e���V���pCPU�f�B�X�N���v�^

	Texture							m_texture;

	D3D12_VERTEX_BUFFER_VIEW		m_VBV;	// ���_�o�b�t�@�r���[
	D3D12_INDEX_BUFFER_VIEW			m_ibv;	// �C���f�b�N�X�o�b�t�@�r���[
	D3D12_VIEWPORT					m_viewport;	// �r���[�|�[�g
	D3D12_RECT						m_scissor;	// �V�U�[��`
	ConstantBufferView<Transform>	m_CBV[FrameCount * 2];
	float							m_rotateAngle;	// ��]�p
};

