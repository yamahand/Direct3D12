#pragma once

#include <d3d12.h>
#include <atomic>
#include <ComPtr.h>
#include <Pool.h>

class DescriptorHandle {
public :
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU;
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU;

	bool HasCPU() const { return handleCPU.ptr != 0; }
	bool HasGPU() const { return handleGPU.ptr != 0; }
};

/// <summary>
/// ディスクリプターテーブル
/// </summary>
class DescriptorPool{
public:
	/// <summary>
	/// ディスクリプターテーブル生成
	/// </summary>
	/// <param name="pDevice"></param>
	/// <param name="pDesc"></param>
	/// <param name="ppPool"></param>
	/// <returns></returns>
	static bool Create(ID3D12Device* pDevice, const D3D12_DESCRIPTOR_HEAP_DESC* pDesc, DescriptorPool** ppPool);

	/// <summary>
	/// 参照カウントの追加
	/// </summary>
	void AddRed();

	/// <summary>
	/// 開放処理
	/// </summary>
	void Release();

	/// <summary>
	/// 参照カウント数の取得
	/// </summary>
	/// <returns>参照カウント数</returns>
	uint32_t GetCount() const;

	/// <summary>
	/// ディスクリプタハンドルを割り当て
	/// </summary>
	/// <returns>割り当てられたディスクリプタハンドル</returns>
	DescriptorHandle* AllocHandle();
};
