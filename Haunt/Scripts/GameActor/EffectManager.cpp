#include "EffectManager.h"
#include "Effect.h"

namespace
{
	const int MaxEffectNum = 5;
}

EffectManager::EffectManager()
	: Actor(L"EffectManager")
{
	for (int i = 0; i < MaxEffectNum; i++) {
		auto effect = std::make_unique<Effect>(L"Contents/Textures/GetCoin.png", 1);
		effect->Initialize(1.0f, 14, 1);
		this->AddChild(std::move(effect));
	}
}

HRESULT EffectManager::CreateObject()
{
	if (FAILED(Actor::CreateObject()))
		return E_FAIL;

	return S_OK;
}

HRESULT EffectManager::PlayEffect(CXMVECTOR location, const float spanTime, const float startTime)
{
	auto it = m_Children.begin();
	while (it != m_Children.end()){
		// 現在未使用のエフェクトを探す
		if ((*it)->GetStatus() == NOUSE) {
			// エフェクトの再生を依頼する
			dynamic_cast<Effect*>((*it).get())->PlayEffect(location, spanTime, startTime);
			return S_OK;
		}
		it++;
	}

	return E_FAIL;
}

void EffectManager::FrameMove(const double& time, const float& fElapsedTime)
{
	Actor::FrameMove(time, fElapsedTime);
}

void EffectManager::FrameRender(const double& time, const float& fElapsedTime)
{
	Actor::FrameRender(time, fElapsedTime);
}