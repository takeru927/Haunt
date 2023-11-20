#pragma once
#include "GameFrame/ModelActor.h"

class EnemySimple : public ModelActor
{
private:
	float m_Speed;

public:
	EnemySimple(Shader* shader);
	~EnemySimple() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;
};