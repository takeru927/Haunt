#pragma once
#include <unordered_map>
#include <unordered_set>
#include "StaticMesh.h"
#include "SkeltalMesh.h"

class MeshManager
{
private:
	MeshManager()
	{
	}

	static MeshManager* instance;

public:
	static void CreateInstance()
	{
		instance = new MeshManager();
	}

	static void DeleteInstance()
	{
		delete instance;
	}

	static MeshManager* GetInstance()
	{
		return instance;
	}

private:
	std::unordered_set<std::wstring> m_CreateList;
	std::unordered_map<std::wstring, std::unique_ptr<StaticMesh>>  m_StaticMeshs;
	std::unordered_map<std::wstring, std::unique_ptr<SkeltalMesh>> m_SkeltalMeshs;

public:
	~MeshManager() = default;

	void ReserveCreateMesh(const LPCWSTR fileName);
	void ReleaseUnusedMesh();

	StaticMesh* GetStaticMeshObject(const LPCWSTR fileName);
	SkeltalMesh* GetSkeltalMeshObject(const LPCWSTR fileName);
};