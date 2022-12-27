#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include "Pool.h"

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

struct Material_ {
	DirectX::XMFLOAT3 diffuse;		//!< 拡散反射成分
	DirectX::XMFLOAT3 specular;		//!< 鏡面反射成分
	float alpha;					//!< 透過成分
	float shininess;				//!< 鏡面反射強度
	std::string diffuseMap;			//!< テクスチャファイルパス
};

struct Mesh {
	std::vector<MeshVertex>	vertices;	//!< 頂点データ
	std::vector<uint32_t>	indices;	//!< 頂点インデックス
	uint32_t				materialId;	//!< マテリアル番号
};

/// <summary>
/// メッシュをロード
/// </summary>
/// <param name="fileName">ファイルパス</param>
/// <param name="meshes">メッシュ格納先</param>
/// <param name="materials">マテリアル格納先</param>
/// <returns>ロードに成功したらtrue</returns>
bool LoadMesh(
	const wchar_t* fileName,
	std::vector<Mesh>& meshes,
	std::vector<Material_>& materials
);

struct AAA {
	Pool<int32_t> aaa;
};