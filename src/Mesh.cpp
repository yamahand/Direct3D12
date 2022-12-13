#include "Mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <codecvt>
#include <cassert>

#pragma comment(lib, "assimp-vc142-mtd.lib")

namespace {

	std::string ToUTF8(const std::wstring& value) {
		auto length = WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, nullptr, 0, nullptr, nullptr);
		auto buffer = new char[length];

		WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, buffer, length, nullptr, nullptr);

		std::string result(buffer);
		delete[] buffer;
		return result;
	}

	class MeshLoader {
	public:
		MeshLoader();
		~MeshLoader();

		bool Load(
			const wchar_t* fileName,
			std::vector<Mesh>& meshes,
			std::vector<Material>& materials
		);

	private:
		void ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh);
		void ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial);
	};

	MeshLoader::MeshLoader(){}
	MeshLoader::~MeshLoader(){}

	bool MeshLoader::Load(
		const wchar_t* fileName,
		std::vector<Mesh>& meshes,
		std::vector<Material>& materials
	) {
		if (fileName == nullptr) return false;

		// watch_t からchar型(utf-8)に変換
		auto path = ToUTF8(fileName);

		Assimp::Importer importer;
		int flag = 0;
		flag |= aiProcess_Triangulate;
		flag |= aiProcess_PreTransformVertices;
		flag |= aiProcess_CalcTangentSpace;
		flag |= aiProcess_GenSmoothNormals;
		flag |= aiProcess_GenUVCoords;
		flag |= aiProcess_RemoveRedundantMaterials;
		flag |= aiProcess_OptimizeMeshes;

		// ファイル読み込み
		auto pScene = importer.ReadFile(path, flag);

		if (pScene == nullptr) {
			return false;
		}

		// メッシュのメモリを確保
		meshes.clear();
		meshes.resize(pScene->mNumMeshes);

		// メッシュデータを変換
		for (size_t i = 0; i < meshes.size(); i++)
		{
			const auto pMesh = pScene->mMeshes[i];
			ParseMesh(meshes[i], pMesh);
		}

		// マテリアルのメモリを確保
		materials.clear();
		materials.resize(pScene->mNumMaterials);

		// マテリアルデータを変換
		for (size_t i = 0; i < materials.size(); i++)
		{
			const auto pMaterial = pScene->mMaterials[i];
			ParseMaterial(materials[i], pMaterial);
		}

		pScene = nullptr;

		return true;
	}

	/// <summary>
	/// メッシュデータを解析
	/// </summary>
	/// <param name="dstMesh"></param>
	/// <param name="pSrcMesh"></param>
	void MeshLoader::ParseMesh(Mesh& dstMesh, const aiMesh* pSrcMesh) {
		// マテリアル番号を設定
		dstMesh.materialId = pSrcMesh->mMaterialIndex;

		aiVector3D zero3D(0.0f, 0.0f, 0.0f);

		// 頂点データのメモリを確保
		dstMesh.vertices.resize(pSrcMesh->mNumVertices);

		for (auto i = 0U; i < pSrcMesh->mNumVertices; i++)
		{
			auto pPosition = &(pSrcMesh->mVertices[i]);
			auto pNormal = &(pSrcMesh->mNormals[i]);
			auto PTexCoord = (pSrcMesh->HasTextureCoords(0)) ? &(pSrcMesh->mTextureCoords[0][1]) : &zero3D;
			auto pTangent = (pSrcMesh->HasTangentsAndBitangents() ) ? &(pSrcMesh->mTangents[1]) : &zero3D;

			dstMesh.vertices[i] = MeshVertex(
				DirectX::XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
				DirectX::XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
				DirectX::XMFLOAT2(PTexCoord->x, PTexCoord->y),
				DirectX::XMFLOAT3(pTangent->x, pTangent->y, pTangent->z)
			);
		}

		// 頂点インデックスのメモリを確保
		dstMesh.indices.resize(pSrcMesh->mNumFaces * 3);

		for (auto i = 0u; i < pSrcMesh->mNumFaces; i++)
		{
			const auto& face = pSrcMesh->mFaces[i];
			assert(face.mNumIndices == 3);	// 三角形化しているので必ず3になっているはず

			dstMesh.indices[i * 3 + 0] = face.mIndices[0];
			dstMesh.indices[i * 3 + 1] = face.mIndices[1];
			dstMesh.indices[i * 3 + 2] = face.mIndices[2];
		}
	}

	/// <summary>
	/// マテリアルデータを解析
	/// </summary>
	/// <param name="dstMaterial"></param>
	/// <param name="pSrcMaterial"></param>
	void MeshLoader::ParseMaterial(Material& dstMaterial, const aiMaterial* pSrcMaterial) {
		// 拡散反射
		{
			aiColor3D color(0.0f, 0.0f, 0.0f);
			if (pSrcMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, color) == aiReturn_SUCCESS) {
				dstMaterial.diffuse.x = color.r;
				dstMaterial.diffuse.y = color.g;
				dstMaterial.diffuse.z = color.b;
			}
			else {
				dstMaterial.diffuse.x = 0.5f;
				dstMaterial.diffuse.y = 0.5f;
				dstMaterial.diffuse.z = 0.5f;
			}
		}

		// 鏡面反射成分
		{
			aiColor3D color(0.0f, 0.0f, 0.0f);
			if (pSrcMaterial->Get(AI_MATKEY_COLOR_SPECULAR, color) == aiReturn_SUCCESS) {
				dstMaterial.specular.x = color.r;
				dstMaterial.specular.y = color.g;
				dstMaterial.specular.z = color.b;
			}
			else {
				dstMaterial.specular.x = 0.0f;
				dstMaterial.specular.y = 0.0f;
				dstMaterial.specular.z = 0.0f;
			}
		}

		// 鏡面反射強度
		{
			auto shininess = 0.0f;
			if (pSrcMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, shininess) == aiReturn_SUCCESS) {
				dstMaterial.shininess = shininess;
			}
			else {
				dstMaterial.shininess = 0.0f;
			}
		}

		// ディフーズ
		{
			aiString path;
			if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == aiReturn_SUCCESS) {
				dstMaterial.diffuseMap = path.C_Str();
			}
			else {
				dstMaterial.diffuseMap.clear();
			}
		}
	}
}

#define FMT_FLOAT3	DXGI_FORMAT_R32G32B32A32_FLOAT
#define FMT_FLOAT2	DXGI_FORMAT_R32G32_FLOAT
#define APPEND		D3D12_APPEND_ALIGNED_ELEMENT
#define IL_VERTEX	D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA

const D3D12_INPUT_ELEMENT_DESC MeshVertex::InputElements[] = {
	{"POSITION", 0, FMT_FLOAT3, 0, APPEND, IL_VERTEX, 0},
	{"NORMAL",	 0, FMT_FLOAT3, 0, APPEND, IL_VERTEX, 0},
	{"TEXCOORD", 0, FMT_FLOAT2, 0, APPEND, IL_VERTEX, 0},
	{"TANGENT",  0, FMT_FLOAT3, 0, APPEND, IL_VERTEX, 0},
};
const D3D12_INPUT_LAYOUT_DESC MeshVertex::InputLayout = {
	MeshVertex::InputElements,
	MeshVertex::InputElementCount
};
static_assert(sizeof(MeshVertex) == 44, "Vertex struct/layout missmatch");

bool LoadMesh(const wchar_t* fileName, std::vector<Mesh>& meshes, std::vector<Material>& materials)
{
	MeshLoader loader;
	return loader.Load(fileName, meshes, materials);
}
