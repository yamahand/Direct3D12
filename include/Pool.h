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
	/// ������
	/// </summary>
	/// <param name="count">�m�ۂ��鐔</param>
	/// <returns>�������ɐ���������true</returns>
	bool Init(uint32_t count) {
		std::lock_guard<std::mutex> guard(m_mutex);

		m_pBuffer = static_cast<uint8_t*>(malloc(sizeof(Item) * (count + 2)));
		if (m_pBuffer == nullptr) {
			return false;
		}

		m_capacity = count;

		// �C���f�b�N�X��U��
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
	/// �I��
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
	/// �m��
	/// </summary>
	/// <typeparam name="T">�m�ۂ����A�C�e���ւ̃|�C���^</typeparam>
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

		// ���������蓖��
		auto val = new((void*)item) T();

		// �������̕K�v������ΌĂяo��
		if (func != nullptr) {
			func(item->m_index, val);
		}

		return val;
	}

	/// <summary>
	/// ���
	/// </summary>
	/// <param name="pValue">�������A�C�e���ւ̃|�C���^</param>
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
	/// ���A�C�e�����擾
	/// </summary>
	/// <returns>���A�C�e����</returns>
	uint32_t GetSize() const {
		return m_capacity;
	}

	/// <summary>
	/// �g�p���̃A�C�e�����擾
	/// </summary>
	/// <returns>�g�p���̃A�C�e����</returns>
	uint32_t GetUsedCount() const {
		return m_count;
	}

	/// <summary>
	/// ���p�\�ȃA�C�e�����̎擾
	/// </summary>
	/// <returns>���p�\�ȃA�C�e����</returns>
	uint32_t GetAvailableCount() const {
		return m_capacity - m_count;
	}

private:
	struct Item {
		T m_value;	//!< �l
		uint32_t m_index = 0;	//!< �C���f�b�N�X;
		Item* m_pNext = nullptr;	//!< ���ւ̃|�C���^
		Item* m_pPrev = nullptr;	//!< �O�ւ̃|�C���^
	};

	/// <summary>
	/// �A�C�e���擾
	/// </summary>
	/// <param name="index">�擾����A�C�e���ւ̃C���f�b�N�X</param>
	/// <returns>�A�C�e���ւ̃|�C���^</returns>
	Item* GetItem(uint32_t index) {
		assert(0 <= index && index <= m_capacity + 2);
		return reinterpret_cast<Item*>(m_pBuffer + sizeof(Item) * index);
	}

	/// <summary>
	/// �A�C�e���Ƀ����������蓖�Ă�
	/// </summary>
	/// <param name="index">�����������蓖�Ă�A�C�e���̃C���f�b�N�X</param>
	/// <returns>�A�C�e���ւ̃|�C���^</returns>
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