#pragma once

#include <d3d12.h>
#include <atomic>
#include <ComPtr.h>
#include <Pool.h>

class DescriptorHandle {
public :
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = { 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = { 0 };

	bool HasCPU() const { return handleCPU.ptr != 0; }
	bool HasGPU() const { return handleGPU.ptr != 0; }
};

/// <summary>
/// ディスクリプターテーブル
/// </summary>
class DescriptorPool{
private:
	DescriptorPool();

	~DescriptorPool();

	DescriptorPool(const DescriptorPool&) = delete;
	void operator = (const DescriptorPool&) = delete;

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
	void AddRef();

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

	/// <summary>
	/// ディスクリプタハンドルを開放する
	/// </summary>
	/// <param name="pHandle">開放するディスクリプタハンドル</param>
	void FreeHandle(DescriptorHandle*& pHandle);

	/// <summary>
	/// 利用可能なハンドル数を取得
	/// </summary>
	/// <returns>利用可能なハンドル数</returns>
	uint32_t GetAvailableHandleCount() const;

	/// <summary>
	/// 割り当て済みハンドル数を取得
	/// </summary>
	/// <returns>割り当て済みハンドル数</returns>
	uint32_t GetAllocatedHandle() const;

	/// <summary>
	/// ハンドル総数を取得
	/// </summary>
	/// <returns>ハンドル総数</returns>
	uint32_t GetHandleCount() const;

	/// <summary>
	/// ディスクリプタヒープを取得
	/// </summary>
	/// <returns>ディスクリプタヒープ</returns>
	ID3D12DescriptorHeap* const GetHeap() const;

private:
	std::atomic<uint32_t> m_refCount;	//!< 参照カウント
	Pool<DescriptorHandle>	m_pool;	//!< ディスクリプタハンドルプール
	ComPtr<ID3D12DescriptorHeap>	m_pHeap;	//!< ディスクリプタヒープ
	uint32_t m_descriptorSize;	//!< ディスクリプタサイズ
};
