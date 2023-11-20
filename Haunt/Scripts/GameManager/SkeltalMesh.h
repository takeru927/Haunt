#pragma once
#include "MeshImporter.h"

class SkeltalMesh
{
private:
	std::vector<MeshAnimNode>				  m_Meshs;
	std::unordered_map<std::string, UINT>	  m_Bones;
	std::unordered_map<std::string, Material> m_Materials;

	int m_MaterialNum;

public:
	SkeltalMesh();
	virtual ~SkeltalMesh() = default;

	HRESULT CreateModel(LPCWSTR fileName);
	void RenderModel(std::function<void(const Material*)> shaderSetting);

	std::unordered_map<std::string, UINT>& GetBones();
};