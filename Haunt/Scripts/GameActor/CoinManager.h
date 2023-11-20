#pragma once
#include "GameFrame/Actor.h"
#include "GameLibrary/Shader.h"

using namespace DirectX;

__declspec(align(16))
class CoinManager : public Actor
{
private:
	std::list<XMVECTOR> m_CoinLocates;

public:
	CoinManager(Shader* shader);
	~CoinManager() = default;

	HRESULT CreateObject() override;

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(CoinManager));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};