#pragma once
#include "GameFrame/Scene.h"

class ResultScene : public Scene
{
private:
	bool  m_isFade;
	float m_Timer;

public:
	ResultScene(Shader* const shader);
	~ResultScene() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void SetStatus(const EActorStatus status) override;
};