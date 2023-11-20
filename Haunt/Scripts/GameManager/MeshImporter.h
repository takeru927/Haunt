#pragma once
#include <unordered_map>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Texture;
};

struct VertexAnim
{
	XMFLOAT3 Pos;
	XMFLOAT3 Normal;
	XMFLOAT2 Texture;
	XMFLOAT4 BoneIndex;
	XMFLOAT4 Weight;
};

struct Material
{
	XMFLOAT4 Ambient;
	XMFLOAT4 Diffuse;
	XMFLOAT4 Emissive;

	ComPtr<ID3D11ShaderResourceView> Texture;

	Material() {
		ZeroMemory(this, sizeof(Material));
	}
};

struct MeshNode
{
	CHAR				 MaterialName[256];
	std::vector<Vertex>  Vertices;
	std::vector<DWORD>	 Indices;
	ComPtr<ID3D11Buffer> VertexBuffer;
	ComPtr<ID3D11Buffer> IndexBuffer;

	MeshNode() {
		VertexBuffer = nullptr;
		IndexBuffer = nullptr;
	}
};

struct MeshAnimNode
{
	CHAR					 MaterialName[256];
	std::vector<VertexAnim>  Vertices;
	std::vector<DWORD>		 Indices;
	ComPtr<ID3D11Buffer>	 VertexBuffer;
	ComPtr<ID3D11Buffer>	 IndexBuffer;

	MeshAnimNode() {
		VertexBuffer = nullptr;
		IndexBuffer = nullptr;
	}
};

namespace MeshImporter
{
	HRESULT LoadObjMesh(LPCWSTR fileName,
						std::vector<MeshNode>& meshs, 
						std::unordered_map<std::string, Material>& materials,
						int& materialNum);

	HRESULT LoadFbxMesh(LPCWSTR fileName,
						std::vector<MeshNode>& meshs,
						std::unordered_map<std::string, Material>& materials,
						int& materialNum);

	HRESULT LoadFbxAnimMesh(LPCWSTR fileName,
							std::vector<MeshAnimNode>& meshs,
							std::unordered_map<std::string, UINT>& bones,
							std::unordered_map<std::string, Material>& materials,
							int& materialNum);
}