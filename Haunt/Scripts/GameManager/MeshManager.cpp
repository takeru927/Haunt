#include "MeshManager.h"

MeshManager* MeshManager::instance = nullptr;

void MeshManager::ReserveCreateMesh(const LPCWSTR fileName)
{
	m_CreateList.insert(fileName);
}

void MeshManager::ReleaseUnusedMesh()
{
	// スタティックメッシュのチェック
	for (auto it = m_StaticMeshs.begin(); it != m_StaticMeshs.end();)
	{
		auto found = m_CreateList.find((*it).first);
		if (found == m_CreateList.end())
		{
			(*it).second.reset();
			it = m_StaticMeshs.erase(it);
		}
		else
			it++;
	}

	// スケルタルメッシュのチェック
	for (auto it = m_SkeltalMeshs.begin(); it != m_SkeltalMeshs.end();)
	{
		auto found = m_CreateList.find((*it).first);
		if (found == m_CreateList.end())
		{
			(*it).second.reset();
			it = m_SkeltalMeshs.erase(it);
		}
		else
			it++;
	}

	m_CreateList.clear();
}

StaticMesh* MeshManager::GetStaticMeshObject(const LPCWSTR fileName)
{
	std::wstring key = fileName;

	auto found = m_StaticMeshs.find(key);

	// データが存在しない
	if (found == m_StaticMeshs.end())
	{
		HRESULT hr = S_OK;
		
		std::unique_ptr<StaticMesh> mesh = std::make_unique<StaticMesh>();
		hr = mesh->CreateModel(fileName);
		if (FAILED(hr)) {
			mesh.reset();
			return nullptr;
		}

		auto res = mesh.get();
		m_StaticMeshs.emplace(key, std::move(mesh));

		return res;
	}

	return found->second.get();
}

SkeltalMesh* MeshManager::GetSkeltalMeshObject(const LPCWSTR fileName)
{
	std::wstring key = fileName;

	auto found = m_SkeltalMeshs.find(key);

	// データが存在しない
	if (found == m_SkeltalMeshs.end())
	{
		HRESULT hr = S_OK;

		std::unique_ptr<SkeltalMesh> mesh = std::make_unique<SkeltalMesh>();
		hr = mesh->CreateModel(fileName);
		if (FAILED(hr)) {
			mesh.reset();
			return nullptr;
		}

		auto res = mesh.get();
		m_SkeltalMeshs.emplace(key, std::move(mesh));

		return res;
	}

	return found->second.get();
}