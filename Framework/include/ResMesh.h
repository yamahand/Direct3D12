#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct ResMaterial {
	DirectX::XMFLOAT3 diffuse;	//!< �g�U���ː���
	DirectX::XMFLOAT3 specular;	//!< ���ʔ��ː���
	float alpha;	//!< ���ߐ���
	float shininess;	//!< ���ʔ��ˋ��x
	std::wstring diffuseMap;	//!< �f�B�t���[�Y�}�b�v�t�@�C���p�X
	std::wstring specularMap;	//!< �X�y�L�����[�}�b�v�t�@�C���p�X
	std::wstring shininessMap;	//!< �V���C�l�X�}�b�v�t�@�C���p�X
	std::wstring normalMap;		//!< �@���}�b�v�t�@�C���p�X
};

class MeshVertex {
public:
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 texCoord;
	DirectX::XMFLOAT3 tangent;

	MeshVertex() = default;

	MeshVertex(
		DirectX::XMFLOAT3 const& pos,
		DirectX::XMFLOAT3 const& nor,
		DirectX::XMFLOAT2 const& tex,
		DirectX::XMFLOAT3 const& tan)
		: position(pos)
		, normal(nor)
		, texCoord(tex)
		, tangent(tan)
	{
	}

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;

private:
	static const int InputElementCount = 4;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct ResMesh {
	std::vector<MeshVertex> vertices;	//!< ���_�f�[�^
	std::vector<uint32_t> indices;		//!< ���_�C���f�b�N�X
	uint32_t materialId;				//!< �}�e���A���ԍ�
};

bool LoadMesh(const wchar_t* fileName, std::vector<ResMesh>& meshes, std::vector<ResMaterial>& materials);

