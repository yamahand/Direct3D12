#pragma once

#include <d3d12.h>
#include <ComPtr.h>

class VertexBuffer {
public:
	VertexBuffer();
	~VertexBuffer();

	VertexBuffer(const VertexBuffer&) = delete;
	void operator = (const VertexBuffer&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="size">頂点バッファサイズ</param>
	/// <param name="stride">1頂点当たりのサイズ</param>
	/// <param name="pInitData">初期化データ</param>
	/// <returns>初期化に成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, size_t size, size_t stride, const void* pInitData = nullptr);

	/// <summary>
	/// 初期化
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="pDevice">デバイス</param>
	/// <param name="size">頂点サイズ</param>
	/// <param name="pInitData">初期化データ</param>
	/// <returns>初期化に成功したらtrue</returns>
	template<typename T>
	bool Init(ID3D12Device* pDevice, size_t size, const T* pInitData) {
		return Init(pDevice, size, sizeof(T), pInitData);
	}

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// メモリマッピング
	/// </summary>
	/// <returns></returns>
	void* Map();

	/// <summary>
	/// メモリマッピングの解除
	/// </summary>
	void Unma();

	/// <summary>
	/// メモリマッピング
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	T* Map() const {
		return reinterpret_cast<T*>(Map());
	}

	/// <summary>
	/// 頂点バッファビューを取得
	/// </summary>
	/// <returns>頂点バッファビュー</returns>
	D3D12_VERTEX_BUFFER_VIEW GetView() const;

private:
	ComPtr<ID3D12Resource> m_pVB = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_view = {};
};
