#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <ComPtr.h>
#include <cstdint>

class DescriptorHandle;
class DescriptorPool;

class ColorTarget {
public:
	ColorTarget();
	~ColorTarget();

	ColorTarget(const ColorTarget&) = delete;
	void operator = (const ColorTarget&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="pPoolRTV">ディスクリプタープール(RTV用)</param>
	/// <param name="width">横幅</param>
	/// <param name="height">立幅</param>
	/// <param name="format">ピクセルフォーマット</param>
	/// <param name="useSRGB">SRGBを使用するか</param>
	/// <returns>初期化に成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, uint32_t width, uint32_t height, DXGI_FORMAT format, bool useSRGB);

	/// <summary>
	/// バックバッファを使って初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="pPoolRTV">ディスクリプタプール(RTV用)</param>
	/// <param name="useSRGB">SRGBを使用するか</param>
	/// <param name="index">バックバッファインデックス</param>
	/// <param name="pSwapChain">スワップチェイン</param>
	/// <returns>初期化に成功したらtrue</returns>
	bool InitFromBackBuffer(ID3D12Device* pDevice, DescriptorPool* pPoolRTV, bool useSRGB, uint32_t index, IDXGISwapChain* pSwapChain);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// ディスクリプタハンドル(RTV用)を取得
	/// </summary>
	/// <returns>ディスクリプタハンドル(RTV用)</returns>
	DescriptorHandle* GetHandleRTV() const;

	/// <summary>
	///  リソース取得
	/// </summary>
	/// <returns>リソース</returns>
	ID3D12Resource* GetResource() const;

	/// <summary>
	/// リソース設定を取得
	/// </summary>
	/// <returns>リソース設定</returns>
	D3D12_RESOURCE_DESC GetDesc() const;

	/// <summary>
	/// レンダーターゲットビューの設定を取得
	/// </summary>
	/// <returns>レンダーターゲットビューの設定</returns>
	D3D12_RENDER_TARGET_VIEW_DESC GetViewDesc() const;

private:
	ComPtr<ID3D12Resource> m_pTarget = nullptr;
	DescriptorHandle* m_pHandleRTV = nullptr;
	DescriptorPool* m_pPoolRTV = nullptr;
	D3D12_RENDER_TARGET_VIEW_DESC m_viewDesc = {};

};
