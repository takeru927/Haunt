#pragma once
#include "GameFrame/Scene.h"
#include "GameLibrary/CutScene.h"

class TitleScene : public Scene
{
private:
	std::unique_ptr<CutScene> m_StartCut;

public:
	TitleScene(Shader* const shader);
	~TitleScene() = default;

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