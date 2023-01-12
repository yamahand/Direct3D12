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
	/// ������
	/// </summary>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="size">���_�o�b�t�@�T�C�Y</param>
	/// <param name="stride">1���_������̃T�C�Y</param>
	/// <param name="pInitData">�������f�[�^</param>
	/// <returns>�������ɐ���������true</returns>
	bool Init(ID3D12Device* pDevice, size_t size, size_t stride, const void* pInitData = nullptr);

	/// <summary>
	/// ������
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="pDevice">�f�o�C�X</param>
	/// <param name="size">���_�T�C�Y</param>
	/// <param name="pInitData">�������f�[�^</param>
	/// <returns>�������ɐ���������true</returns>
	template<typename T>
	bool Init(ID3D12Device* pDevice, size_t size, const T* pInitData) {
		return Init(pDevice, size, sizeof(T), pInitData);
	}

	/// <summary>
	/// �I������
	/// </summary>
	void Term();

	/// <summary>
	/// �������}�b�s���O
	/// </summary>
	/// <returns></returns>
	void* Map();

	/// <summary>
	/// �������}�b�s���O�̉���
	/// </summary>
	void Unma();

	/// <summary>
	/// �������}�b�s���O
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <returns></returns>
	template<typename T>
	T* Map() const {
		return reinterpret_cast<T*>(Map());
	}

	/// <summary>
	/// ���_�o�b�t�@�r���[���擾
	/// </summary>
	/// <returns>���_�o�b�t�@�r���[</returns>
	D3D12_VERTEX_BUFFER_VIEW GetView() const;

private:
	ComPtr<ID3D12Resource> m_pVB = nullptr;
	D3D12_VERTEX_BUFFER_VIEW m_view = {};
};
