#pragma once
#include "GameFrame/ModelActor.h"

class Coin : public ModelActor
{
private:

public:
	Coin(Shader* shader);
	~Coin() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnCollide(SceneActor* other) override;
};