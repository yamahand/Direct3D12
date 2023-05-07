#pragma once


#ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
#endif

#ifndef NOMINMAX
    #define NOMINMAX
#endif

#include <windows.h>

#include <array>
#include <bitset>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <format>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui/imgui.h>
#include <imgui/imgui_impl_win32.h>
#include <imgui/imgui_impl_dx12.h>


#ifdef _WIN64

#include <wrl.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <dxgi1_6.h>
// #include <d3dx12.h>
// #include <DirectXTex.h>
#include <DirectXMath.h>

#ifdef _DEBUG
    #define DX12_ENABLE_DEBUG_LAYER
#endif


#define D3D12_GPU_VIRTUAL_ADDRESS_NULL      ((D3D12_GPU_VIRTUAL_ADDRESS)0)
#define D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN   ((D3D12_GPU_VIRTUAL_ADDRESS)-1)

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

#ifdef DX12_ENABLE_DEBUG_LAYER
    #include <dxgidebug.h>
    #pragma comment(lib, "dxguid.lib")
#endif

#endif
