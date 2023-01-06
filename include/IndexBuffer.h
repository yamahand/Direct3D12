#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <cstdint>

class IndexBuffer {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	IndexBuffer();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~IndexBuffer();

	IndexBuffer(const IndexBuffer&) = delete;
	void operator = (const IndexBuffer&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="size">インデックスバッファサイズ</param>
	/// <param name="pInitData">初期化データ</param>
	/// <returns>初期化成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, size_t size, const uint32_t* pInitData = nullptr);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// メモリマッピング
	/// </summary>
	/// <returns></returns>
	uint32_t* Map();

	/// <summary>
	/// メモリマッピング解除
	/// </summary>
	void Unmap();

	/// <summary>
	/// インデックスバッファビューを取得
	/// </summary>
	/// <returns>インデックスバッファビュー</returns>
	D3D12_INDEX_BUFFER_VIEW GetView() const;

private:
	ComPtr<ID3D12Resource> m_pIB = nullptr;
	D3D12_INDEX_BUFFER_VIEW m_view = {};
};
