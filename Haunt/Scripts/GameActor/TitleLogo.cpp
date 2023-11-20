#include "TitleLogo.h"
#include "GameMain.h"
#include "GameLibrary/DirectDevice.h"

TitleLogo::TitleLogo()
	: SpriteActor(L"TitleLogo"), m_Timer(0.0f), m_ChangeAlpha(0.0f)
{
	m_Status = RENDERONLY;

	Initialize(500.0f, 370.0f, 0.1f, false, ECtrlPoint::CENTER, ECtrlPoint::CENTER);
	SetLocation({ 0.0f, -200.0f });
}

HRESULT TitleLogo::CreateObject()
{
	HRESULT hr = S_OK;

	hr = CreateTextureResourse(L"Contents/Textures/TitleLogo.png");
	if (FAILED(hr))
		return hr;

	hr = SpriteActor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void TitleLogo::FrameMove(const double& time, const float& fElapsedTime)
{
	if (m_Timer <= 0.0f)
		m_Status = DEAD;

	m_Timer -= fElapsedTime;
	m_Color.w -= m_ChangeAlpha;
	if (m_Color.w <= 0.0f)
		m_Color.w = 0.0f;

	SpriteActor::FrameMove(time, fElapsedTime);
}

void TitleLogo::SetStatus(const EActorStatus status)
{
	// •ÏX‚ðŽó‚¯•t‚¯‚È‚¢
	return;
}

void TitleLogo::FadeOut(float fadeTime)
{
	if (fadeTime <= 0.0f) fadeTime = 0.000001f;

	m_Status = ACTIVE;
	m_Timer  = fadeTime;
	m_ChangeAlpha = 1.0f / (GAME->GetFPS() * fadeTime);
}