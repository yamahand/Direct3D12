#pragma once

#include <d3d12.h>
#include <ComPtr.h>
#include <cstdint>
#include <vector>


class CommandList {
public:

	CommandList();
	~CommandList();

	CommandList(const CommandList&) = delete;
	void operator = (const CommandList&) = delete;

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="pDevice">デバイス</param>
	/// <param name="type">コマンドリストタイプ</param>
	/// <param name="count">アロケータ数.ダブルバッファの時は2を設定</param>
	/// <returns>初期化に成功したらtrue</returns>
	bool Init(ID3D12Device* pDevice, D3D12_COMMAND_LIST_TYPE type, uint32_t count);

	/// <summary>
	/// 終了処理
	/// </summary>
	void Term();

	/// <summary>
	/// リセット処理を行ったコマンドリストを取得
	/// </summary>
	/// <returns>リセット処理を行ったコマンドリスト</returns>
	ID3D12GraphicsCommandList* Reset();

private:
	ComPtr<ID3D12GraphicsCommandList>	m_pCmdList = nullptr;
	std::vector<ComPtr<ID3D12CommandAllocator>> m_pAllocators = {};
	uint32_t m_index = 0;
};
