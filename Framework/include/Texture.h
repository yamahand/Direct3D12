#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <ResourceUploadBatch.h>

class DescriptorHandle;
class DescriptorPool;

class Texture {
public:
	Texture();
	~Texture();

	Texture(const Texture&) = delete;
	void operator = (const Texture&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="pPool">ディスクリプタプール</param>
	/// <param name="fileName">ファイル名</param>
	/// <param name="useSRGB">SRGBを使用するか</param>
	/// <param name="batch">アップロードバッチ</param>
	/// <returns>成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, const wchar_t* fileName, bool useSRGB, DirectX::ResourceUploadBatch& batch);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="pPool">ディスクリプタプール</param>
	/// <param name="pDesc">でスクリプタ</param>
	/// <param name="useSRGB">SRGBを使用するか</param>
	/// <param name="isCube">キューブマップか</param>
	/// <returns>成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPool, const D3D12_RESOURCE_DESC* pDesc, bool useSRGB, bool isCube);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// CPUディスクリプタハンドル取得
	/// </summary>
	/// <returns>CPUディスクリプタハンドル</returns>
	D3D12_CPU_DESCRIPTOR_HANDLE GetHandleCPU();

	/// <summary>
	/// GPUディスクリプタハンドル取得
	/// </summary>
	/// <returns>CPUディスクリプタハンドル</returns>
	D3D12_GPU_DESCRIPTOR_HANDLE GetHandleGPU();

private:
	/// <summary>
	/// シェーダリソースビューの設定を取得
	/// </summary>
	/// <param name="isCube">キューブマップか</param>
	/// <returns>シェーダーリソースビューの設定</returns>
	D3D12_SHADER_RESOURCE_VIEW_DESC GetViewDesc(bool isCube);

private:
	ComPtr<ID3D12Resource> m_pTex;
	DescriptorHandle* m_pHandle;
	DescriptorPool* m_pPool;
};
