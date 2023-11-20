#pragma once
#include "GameFrame/TextActor.h"

class RemainingText : public TextActor
{
private:

public:
	RemainingText();
	~RemainingText() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;
};