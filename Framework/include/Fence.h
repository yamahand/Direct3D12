#pragma once

#include <d3d12.h>
#include <ComPtr.h>

class Fence {
public:
	/// <summary>
	/// コンストラクタ
	/// </summary>
	Fence();

	/// <summary>
	/// デストラクタ
	/// </summary>
	~Fence();

	Fence(const Fence&) = delete;
	void operator = (const Fence&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <returns>初期化成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// シグナル状態になるまで指定時間待機
	/// </summary>
	/// <param name="pQueue">コマンドキュー</param>
	/// <param name="timeout">タイムアウト時間</param>
	void Wait(ID3D12CommandQueue* pQueue, UINT timeout);

	/// <summary>
	/// シグナル状態になるまで待機
	/// </summary>
	/// <param name="pQueue">コマンドキュー</param>
	void Sync(ID3D12CommandQueue* pQueue);

private:
	ComPtr<ID3D12Fence>	m_pFence = nullptr;
	HANDLE m_event = {};
	UINT m_counter = 0;
};
