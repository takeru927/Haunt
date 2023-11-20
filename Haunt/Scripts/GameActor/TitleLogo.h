#pragma once
#include "GameFrame/SpriteActor.h"

class TitleLogo : public SpriteActor
{
private:
	float m_Timer;
	float m_ChangeAlpha;

public:
	TitleLogo();
	~TitleLogo() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void SetStatus(const EActorStatus status) override;

	void FadeOut(float fadeTime);
};