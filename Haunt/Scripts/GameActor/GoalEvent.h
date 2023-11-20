#pragma once
#include "GameFrame/SceneActor.h"
#include "GameLibrary/CutScene.h"

class GoalEvent : public SceneActor
{
private:
	CutScene* m_GoalCut;

public:
	GoalEvent(CutScene* cut);
	~GoalEvent() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;

	void SetStatus(const EActorStatus status) override;
	void OnCollide(SceneActor* other) override;
};