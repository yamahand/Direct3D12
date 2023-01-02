#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <cstdint>

class DescriptorHandle;
class DescriptorPool;

class DepthTarget {
public:
	DepthTarget();
	~DepthTarget();

	DepthTarget(const DepthTarget&) = delete;
	void operator = (const DepthTarget&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="pPoolDSV">ディスクリプタープール(DSV用)</param>
	/// <param name="width">横幅</param>
	/// <param name="height">立幅</param>
	/// <param name="format">ピクセルフォーマット</param>
	/// <returns>初期化に成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolDSV, uint32_t width, uint32_t height, DXGI_FORMAT format);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// ディスクリプタハンドル(DSV用)を取得
	/// </summary>
	/// <returns>ディスクリプタハンドル(DSV用)</returns>
	DescriptorHandle* GetHandleDSV() const;

	/// <summary>
	/// リソースを取得
	/// </summary>
	/// <returns>リソース</returns>
	ID3D12Resource* GetResource() const;

	/// <summary>
	/// リソース設定を取得
	/// </summary>
	/// <returns>リソース設定</returns>
	D3D12_RESOURCE_DESC GetDesc() const;

	/// <summary>
	/// 深度ステンシルビューの設定を取得
	/// </summary>
	/// <returns>深度ステンシルビューの設定</returns>
	D3D12_DEPTH_STENCIL_VIEW_DESC GetViewDesc() const;

private:
	ComPtr<ID3D12Resource>	m_pTarget = nullptr;
	DescriptorHandle* m_pHandleDSV = nullptr;
	DescriptorPool* m_pPoolDSV = nullptr;
	D3D12_DEPTH_STENCIL_VIEW_DESC m_viewDesc = {};

};
