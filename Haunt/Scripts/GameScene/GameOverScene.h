#pragma once
#include "GameFrame/Scene.h"

class GameOverScene : public Scene
{
private:
	bool  m_isFade;
	float m_Timer;

public:
	GameOverScene(Shader* const shader);
	~GameOverScene() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void SetStatus(const EActorStatus status) override;
};