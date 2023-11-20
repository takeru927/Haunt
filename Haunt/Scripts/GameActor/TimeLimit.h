#pragma once
#include "GameFrame/TextActor.h"

class TimeLimit : public TextActor
{
private:
	float m_Time;

public:
	TimeLimit();
	~TimeLimit() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;
};