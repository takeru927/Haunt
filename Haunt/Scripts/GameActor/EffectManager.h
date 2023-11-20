#pragma once
#include "GameFrame/Actor.h"

using namespace DirectX;

class EffectManager : public Actor
{
private:

public:
	EffectManager();
	~EffectManager() = default;

	HRESULT CreateObject() override;

	HRESULT PlayEffect(CXMVECTOR location, const float spanTime = 0.033333f, const float startTime = 0.0f);

	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;
};