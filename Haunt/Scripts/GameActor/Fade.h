#pragma once
#include "GameFrame/SpriteActor.h"
#include "GameLibrary/SpriteShader.h"

namespace
{
	enum EFadeMode {
		NONE,
		FADEIN,
		FADEOUT
	};
}

class Fade : public SpriteActor
{
private:
	float m_FadeTime;
	float m_ChangeAlpha;

	EFadeMode m_Mode;

public:
	Fade();
	~Fade() = default;

	void FrameMove(const double& time, const float& fElapsedTime) override;

	void SetStatus(const EActorStatus status) override;

	void FadeIn(float fadeTime);
	void FadeOut(float fadeTime);
};