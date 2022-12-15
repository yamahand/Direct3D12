#pragma once

#include <cstdint>
#include <mutex>
#include <cassert>
#include <functional>

template<typename T>
class Pool
{
public:
	Pool() {

	}

	~Pool() {
		Term();
	}

	/// <summary>
	/// 初期化
	/// </summary>
	/// <param name="count">確保する数</param>
	/// <returns>初期化に成功したらtrue</returns>
	bool Init(uint32_t count) {
		std::lock_guard<std::mutex> guard(m_mutex);

		m_pBuffer = static_cast<uint8_t*>(malloc(sizeof(Item) * (count + 2)));
		if (m_pBuffer == nullptr) {
			return false;
		}

		m_capacity = count;

		// インデックスを振る
		for (auto i = 2u, j = 0u; i < m_count + 2; i++, j++)
		{
			auto item = GetItem(i);
			item->m_index = j;
		}

		m_pActive = GetItem(0);
		m_pActive->m_pPrev = m_pActive->m_pNext = m_pActive;
		m_pActive->m_index = uint32_t(-1);

		m_pFree = GetItem(1);
		m_pFree->m_index = uint32_t(-2);

		for (auto i = 1u; i < m_capacity + 2; i++)
		{
			GetItem(i)->m_pPrev = nullptr;
			GetItem(i)->m_pNext = GetItem(i + 1);
		}

		GetItem(m_capacity + 1)->m_pPrev = m_pFree;

		m_count = 0;

		return true;
	}

	/// <summary>
	/// 終了
	/// </summary>
	void Term() {
		std::lock_guard<std::mutex> guard(m_mutex);

		if (m_pBuffer) {
			free(m_pBuffer);
			m_pBuffer = nullptr;
		}

		m_pActive = nullptr;
		m_pFree = nullptr;
		m_capacity = 0;
		m_count = 0;
	}

	/// <summary>
	/// 確保
	/// </summary>
	/// <typeparam name="T">確保したアイテムへのポインタ</typeparam>
	T* Alloc(std::function<void(uint32_t, T*) > func = nullptr) {
		std::lock_guard<std::mutex> guard(m_mutex);

		if (m_pFree->m_pNext == m_pFree || m_count + 1 > m_capacity) {
			return nullptr;
		}

		auto item = m_pFree->m_pNext;
		m_pFree->m_pNext = item->m_pNext;

		item->m_pPrev = m_pActive->m_pPrev;
		item->m_pNext = m_pActive;
		item->m_pPrev->m_pNext = item->m_pNext->m_pPrev = item;

		++m_count;

		// メモリ割り当て
		auto val = new((void*)item) T();

		// 初期化の必要があれば呼び出す
		if (func != nullptr) {
			func(item->m_index, val);
		}

		return val;
	}

	/// <summary>
	/// 解放
	/// </summary>
	/// <param name="pValue">解放するアイテムへのポインタ</param>
	void Free(T* pValue) {
		if (pValue == nullptr) {
			return;
		}

		std::lock_guard<std::mutex> guard(m_mutex);

		auto item = reinterpret_cast<Item*>(pValue);

		item->m_pPrev->m_pNext = item->m_pNext;
		item->m_pNext->m_pPrev = item->m_pPrev;

		item->m_pPrev = nullptr;
		item->m_pNext = m_pFree->m_pNext;

		m_pFree->m_pNext = item;
		--m_count;
	}

	/// <summary>
	/// 総アイテム数取得
	/// </summary>
	/// <returns>総アイテム数</returns>
	uint32_t GetSize() const {
		return m_capacity;
	}

	/// <summary>
	/// 使用中のアイテム数取得
	/// </summary>
	/// <returns>使用中のアイテム数</returns>
	uint32_t GetUsedCount() const {
		return m_count;
	}

	/// <summary>
	/// 利用可能なアイテム数の取得
	/// </summary>
	/// <returns>利用可能なアイテム数</returns>
	uint32_t GetAvailableCount() const {
		return m_capacity - m_count;
	}

private:
	struct Item {
		T m_value;	//!< 値
		uint32_t m_index = 0;	//!< インデックス;
		Item* m_pNext = nullptr;	//!< 次へのポインタ
		Item* m_pPrev = nullptr;	//!< 前へのポインタ
	};

	/// <summary>
	/// アイテム取得
	/// </summary>
	/// <param name="index">取得するアイテムへのインデックス</param>
	/// <returns>アイテムへのポインタ</returns>
	Item* GetItem(uint32_t index) {
		assert(0 <= index && index <= m_capacity + 2);
		return reinterpret_cast<Item*>(m_pBuffer + sizeof(Item) * index);
	}

	/// <summary>
	/// アイテムにメモリを割り当てる
	/// </summary>
	/// <param name="index">メモリを割り当てるアイテムのインデックス</param>
	/// <returns>アイテムへのポインタ</returns>
	Item* AssignItem(uint32_t index) {
		assert(0 <= index && index <= m_capacity + 2);
		auto buf = (m_pBuffer + sizeof(Item) * index);
		return new(buf) Item;
	}

	Pool(const Pool&) = delete;
	void operator = (const Pool&) = delete;

private:
	uint8_t* m_pBuffer = nullptr;
	Item* m_pActive = nullptr;
	Item* m_pFree = nullptr;
	uint32_t m_capacity = 0;
	uint32_t m_count = 0;
	std::mutex m_mutex = {};
};