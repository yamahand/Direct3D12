#include "ResMesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <codecvt>
#include <cassert>


namespace {

	std::string ToUTF8(const std::wstring& value) {
		auto length = WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, nullptr, 0, nullptr, nullptr);
		auto buffer = new char[length];

		WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, buffer, length, nullptr, nullptr);

		std::string result(buffer);
		delete[] buffer;
		buffer = nullptr;

		return result;
	}

	std::wstring Convert(const aiString& path) {
		wchar_t temp[256] = {};
		size_t size;
		mbstowcs_s(&size, temp, path.C_Str(), 256);
		return std::wstring(temp);
	}

	class MeshLoader {
	public:
		MeshLoader();
		~MeshLoader();

		bool Load(
			const wchar_t* fileName,
			std::vector<ResMesh>& meshes,
			std::vector<ResMaterial>& materials
		);

	private:
		const aiScene* m_pScene = nullptr;

	private:
		void ParseMesh(ResMesh& dstMesh, const aiMesh* pSrcMesh);
		void ParseMaterial(ResMaterial& dstMaterial, const aiMaterial* pSrcMaterial);
	};

	MeshLoader::MeshLoader()
		: m_pScene(nullptr)
	{}

	MeshLoader::~MeshLoader()
	{}

	bool MeshLoader::Load(
		const wchar_t* fileName,
		std::vector<ResMesh>& meshes,
		std::vector<ResMaterial>& materials
	) {
		if (fileName == nullptr) return false;

		// watch_t ����char�^(utf-8)�ɕϊ�
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

		// �t�@�C���ǂݍ���
		m_pScene = importer.ReadFile(path, flag);

		if (m_pScene == nullptr) {
			return false;
		}

		// ���b�V���̃��������m��
		meshes.clear();
		meshes.resize(m_pScene->mNumMeshes);

		// ���b�V���f�[�^��ϊ�
		for (size_t i = 0; i < meshes.size(); i++)
		{
			const auto pMesh = m_pScene->mMeshes[i];
			ParseMesh(meshes[i], pMesh);
		}

		// �}�e���A���̃��������m��
		materials.clear();
		materials.resize(m_pScene->mNumMaterials);

		// �}�e���A���f�[�^��ϊ�
		for (size_t i = 0; i < materials.size(); i++)
		{
			const auto pMaterial = m_pScene->mMaterials[i];
			ParseMaterial(materials[i], pMaterial);
		}

		// �s�v�ɂȂ����̂ŃN���A
		importer.FreeScene();
		m_pScene = nullptr;

		return true;
	}

	/// <summary>
	/// ���b�V���f�[�^�����
	/// </summary>
	/// <param name="dstMesh"></param>
	/// <param name="pSrcMesh"></param>
	void MeshLoader::ParseMesh(ResMesh& dstMesh, const aiMesh* pSrcMesh) {
		// �}�e���A���ԍ���ݒ�
		dstMesh.materialId = pSrcMesh->mMaterialIndex;

		aiVector3D zero3D(0.0f, 0.0f, 0.0f);

		// ���_�f�[�^�̃��������m��
		dstMesh.vertices.resize(pSrcMesh->mNumVertices);

		for (auto i = 0U; i < pSrcMesh->mNumVertices; i++)
		{
			auto pPosition = &(pSrcMesh->mVertices[i]);
			auto pNormal = &(pSrcMesh->mNormals[i]);
			auto PTexCoord = (pSrcMesh->HasTextureCoords(0)) ? &(pSrcMesh->mTextureCoords[0][i]) : &zero3D;
			auto pTangent = (pSrcMesh->HasTangentsAndBitangents()) ? &(pSrcMesh->mTangents[i]) : &zero3D;

			dstMesh.vertices[i] = MeshVertex(
				DirectX::XMFLOAT3(pPosition->x, pPosition->y, pPosition->z),
				DirectX::XMFLOAT3(pNormal->x, pNormal->y, pNormal->z),
				DirectX::XMFLOAT2(PTexCoord->x, PTexCoord->y),
				DirectX::XMFLOAT3(pTangent->x, pTangent->y, pTangent->z)
			);
		}

		// ���_�C���f�b�N�X�̃��������m��
		dstMesh.indices.resize(pSrcMesh->mNumFaces * 3);

		for (auto i = 0u; i < pSrcMesh->mNumFaces; i++)
		{
			const auto& face = pSrcMesh->mFaces[i];
			assert(face.mNumIndices == 3);	// �O�p�`�����Ă���̂ŕK��3�ɂȂ��Ă���͂�

			dstMesh.indices[i * 3 + 0] = face.mIndices[0];
			dstMesh.indices[i * 3 + 1] = face.mIndices[1];
			dstMesh.indices[i * 3 + 2] = face.mIndices[2];
		}
	}

	/// <summary>
	/// �}�e���A���f�[�^�����
	/// </summary>
	/// <param name="dstMaterial"></param>
	/// <param name="pSrcMaterial"></param>
	void MeshLoader::ParseMaterial(ResMaterial& dstMaterial, const aiMaterial* pSrcMaterial) {
		// �g�U����
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

		// ���ʔ��ː���
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

		// ���ʔ��ˋ��x
		{
			auto shininess = 0.0f;
			if (pSrcMaterial->Get(AI_MATKEY_SHININESS_STRENGTH, shininess) == aiReturn_SUCCESS) {
				dstMaterial.shininess = shininess;
			}
			else {
				dstMaterial.shininess = 0.0f;
			}
		}

		// �f�B�t�[�Y�}�b�v
		{
			aiString path;
			if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path) == aiReturn_SUCCESS) {
				dstMaterial.diffuseMap = Convert(path);
			}
			else {
				dstMaterial.diffuseMap.clear();
			}
		}

		// �X�y�L�����[�}�b�v
		{
			aiString path;
			if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_SPECULAR(0), path) == aiReturn_SUCCESS) {
				dstMaterial.specularMap = Convert(path);
			}
			else {
				dstMaterial.specularMap.clear();
			}
		}

		// �V���C�l�X�}�b�v
		{
			aiString path;
			if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_SHININESS(0), path)) {
				dstMaterial.shininessMap = Convert(path);
			}
			else {
				dstMaterial.shininessMap.clear();
			}
		}

		// �@���}�b�v
		{
			aiString path;
			if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(0), path)) {
				dstMaterial.normalMap = Convert(path);
			}
			else {
				if (pSrcMaterial->Get(AI_MATKEY_TEXTURE_HEIGHT(0), path)) {
					dstMaterial.normalMap = Convert(path);
				}
				else {
					dstMaterial.normalMap.clear();
				}
			}
		}
	}
}

const D3D12_INPUT_ELEMENT_DESC MeshVertex::InputElements[] = {
	{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"NORMAL",	 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0,	D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
	{"TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
const D3D12_INPUT_LAYOUT_DESC MeshVertex::InputLayout = { MeshVertex::InputElements, MeshVertex::InputElementCount };
static_assert(sizeof(MeshVertex) == 44, "Vertex struct/layout mismatch");

bool LoadMesh(const wchar_t* fileName, std::vector<ResMesh>& meshes, std::vector<ResMaterial>& materials) {
	MeshLoader loader;
	return loader.Load(fileName, meshes, materials);
}