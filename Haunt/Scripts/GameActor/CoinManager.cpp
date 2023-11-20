#include "CoinManager.h"
#include "Coin.h"
#include <ctime>
#include <cstdlib>

namespace
{
	constexpr int coinNum = 5;
}

CoinManager::CoinManager(Shader* shader)
	: Actor(L"CoinManager")
{
	for (int i = 0; i < coinNum; i++) {
		auto coin = std::make_unique<Coin>(shader);
		this->AddChild(std::move(coin));
	}

	m_CoinLocates = {
		{ 439.0f, 2.0f, 414.0f },
		{ 330.0f, 2.0f, 117.0f },
		{  92.0f, 2.0f,-484.0f },
		{-282.0f, 2.0f, 461.0f },
		{-131.5f, 2.0f, 412.0f },
		{ 211.0f, 2.0f,-417.5f },
		{ 188.0f, 2.0f,-100.0f },
	};
}

HRESULT CoinManager::CreateObject()
{
	srand((unsigned int)time(NULL));

	for (auto& coin : m_Children) {
		auto it = m_CoinLocates.begin();

		// ランダムに位置を決定
		int random = rand() % m_CoinLocates.size();
		while (random--) 
			it++;
		dynamic_cast<ModelActor*>(coin.get())->SetLocation(*it);

		// 位置は重複しないようにする
		m_CoinLocates.erase(it);
	}

	if (FAILED(Actor::CreateObject()))
		return E_FAIL;

	return S_OK;
}