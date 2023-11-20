#pragma once
#include "GameFrame/Scene.h"
#include "GameLibrary/AnimationShader.h"
#include "GameLibrary/CutScene.h"
#include "GameActor/Player.h"

class MainScene : public Scene
{
private:
	AnimationShader* m_AnimShader;
	std::unique_ptr<CutScene> m_GoalCut;

	Player* m_Player;

	bool  m_Init;
	bool  m_PlayingChaseBGM;
	float m_Timer;

public:
	MainScene(Shader* const shader, AnimationShader* const animShader);
	~MainScene() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void SetStatus(const EActorStatus status) override;

	XMMATRIX GetCameraView() override;
	XMMATRIX GetCameraLocation() override;

private:
	void LoadCutScene();
	void LoadMap();
};