#pragma once
#include <unordered_map>
#include "MeshImporter.h"

using Microsoft::WRL::ComPtr;

class StaticMesh
{
private:
	std::vector<MeshNode>					  m_Meshs;
	std::unordered_map<std::string, Material> m_Materials;

	int m_MaterialNum;

public:
	StaticMesh();
	virtual ~StaticMesh() = default;

	HRESULT CreateModel(LPCWSTR fileName);
	void RenderModel(std::function<void(const Material*)> shaderSetting);

	bool Intersect(const XMVECTOR rayPos, const XMVECTOR rayDir, const XMMATRIX* const world, float* const dist);
};