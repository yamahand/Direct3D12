#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct MeshVertex {
	DirectX::XMFLOAT3  position;
	DirectX::XMFLOAT3  normal;
	DirectX::XMFLOAT2  texcoord;
	DirectX::XMFLOAT3  tangent;

	MeshVertex() = default;

	MeshVertex(
		DirectX::XMFLOAT3 const& pos,
		DirectX::XMFLOAT3 const& nor,
		DirectX::XMFLOAT2 const& tex,
		DirectX::XMFLOAT3 const& tan ) 
		: position(pos)
		, normal(nor)
		, texcoord(tex)
		, tangent(tan)
	{
	}

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementCount = 4;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct Material {
	DirectX::XMFLOAT3 diffuse;		//!< �g�U���ː���
	DirectX::XMFLOAT3 specular;		//!< ���ʔ��ː���
	float alpha;					//!< ���ߐ���
	float shininess;				//!< ���ʔ��ˋ��x
	std::string diffuseMap;			//!< �e�N�X�`���t�@�C���p�X
};

struct Mesh {
	std::vector<MeshVertex>	vertice;	//!< ���_�f�[�^
	std::vector<uint32_t>	indices;	//!< ���_�C���f�b�N�X
	uint32_t				materialId;	//!< �}�e���A���ԍ�
};

/// <summary>
/// ���b�V�������[�h
/// </summary>
/// <param name="fileName">�t�@�C���p�X</param>
/// <param name="meshes">���b�V���i�[��</param>
/// <param name="materials">�}�e���A���i�[��</param>
/// <returns>���[�h�ɐ���������true</returns>
bool LoadMesh(
	const wchar_t* fileName,
	std::vector<Mesh>& meshes,
	std::vector<Material>& materials
);
