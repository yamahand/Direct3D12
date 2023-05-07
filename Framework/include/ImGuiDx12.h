#pragma once

class ImGuiDx12 {
public:
	template<class T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

public:
	ImGuiDx12() {};
	~ImGuiDx12() {};

	bool Init(ID3D12Device* device, HWND hwnd);
	void ShutDown();

	void BeginFrame();
	void Render(ID3D12Resource* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE handle);
	void EndFramw();

	ID3D12GraphicsCommandList* GetCommandList() const { return cmdList.Get(); }

private:
	ComPtr<ID3D12GraphicsCommandList>   cmdList = nullptr;
	ComPtr<ID3D12CommandAllocator>      cmdAllocator = nullptr;
	ComPtr<ID3D12DescriptorHeap>        srvDescHeap = nullptr;
};
