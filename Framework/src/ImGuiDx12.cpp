#include "ImGuiDx12.h"
#include "DirectXHelpers.h"

bool ImGuiDx12::Init(ID3D12Device* device, HWND hwnd)
{
    auto result = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(cmdAllocator.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        assert(SUCCEEDED(result));
        return false;
    }
    result = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, cmdAllocator.Get(), nullptr, IID_PPV_ARGS(cmdList.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        assert(SUCCEEDED(result));
        return false;
    }

    D3D12_DESCRIPTOR_HEAP_DESC desc = {};
    desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    desc.NumDescriptors = 1;
    desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    result = device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(srvDescHeap.ReleaseAndGetAddressOf()));
    if (FAILED(result)) {
        assert(SUCCEEDED(result));
        return false;
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::CreateContext();
    ImGuiIO& imguiIo = ImGui::GetIO(); (void)imguiIo;
    //imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    //imguiIo.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX12_Init(device, 1, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, srvDescHeap.Get(), srvDescHeap->GetCPUDescriptorHandleForHeapStart(), srvDescHeap->GetGPUDescriptorHandleForHeapStart());

    cmdList->Close();

    return true;
}

void ImGuiDx12::ShutDown()
{
    ImGui_ImplDX12_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void ImGuiDx12::BeginFrame()
{
    ImGui_ImplDX12_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void ImGuiDx12::Render(ID3D12Resource* renderTarget, D3D12_CPU_DESCRIPTOR_HANDLE handle)
{
    // Rendering
    ImGui::Render();

    cmdAllocator->Reset();
    cmdList->Reset(cmdAllocator.Get(), nullptr);

    // 書き込み用リソースバリア設定
    DirectX::TransitionResource(cmdList.Get(),
        renderTarget,
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );

    cmdList->OMSetRenderTargets(1, &handle, false, nullptr);

    ID3D12DescriptorHeap* heaps[] = { srvDescHeap.Get() };
    cmdList->SetDescriptorHeaps(1, heaps);

    ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), cmdList.Get());

    DirectX::TransitionResource(cmdList.Get(),
        renderTarget,
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );
    cmdList->Close();
}

void ImGuiDx12::EndFramw()
{
}
