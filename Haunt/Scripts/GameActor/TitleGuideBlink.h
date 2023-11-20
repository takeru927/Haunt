#pragma once
#include "GameFrame/TextActor.h"

namespace
{
	enum EBlinkState {
		BLINKOUT,
		BLINKIN,
		VISIBLE
	};
}

class TitleGuideBlink : public TextActor
{
private:
	float m_Interval;
	float m_BlinkTime;
	float m_Timer;
	float m_ChangeAlpha;

	EBlinkState  m_BlinkStatus;

public:
	TitleGuideBlink(const float interval, const float blinkTime);
	~TitleGuideBlink() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;
};