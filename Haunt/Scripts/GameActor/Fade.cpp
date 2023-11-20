#include "Fade.h"
#include "GameLibrary/DirectDevice.h"
#include "GameMain.h"

Fade::Fade()
	: SpriteActor(L"Fade"), m_FadeTime(0.0f), m_ChangeAlpha(0.0f), m_Mode(EFadeMode::NONE)
{
	auto screen = D3D->GetScreenViewport();
	SpriteActor::Initialize(screen.Width, screen.Height, 0.0f, true);

	m_Status = NOUSE;
}

void Fade::FrameMove(const double& time, const float& fElapsedTime)
{
	if (m_FadeTime <= 0.0f)
		m_Status = NOUSE;

	m_FadeTime -= fElapsedTime;

	// アルファ値の更新
	if (m_Mode == EFadeMode::FADEIN) {
		m_Color.w -= m_ChangeAlpha;

		if (m_Color.w <= 0.0f)
			m_Color.w = 0.0f;
	}
	else if (m_Mode == EFadeMode::FADEOUT) {
		m_Color.w += m_ChangeAlpha;

		if (m_Color.w >= 1.0f)
			m_Color.w = 1.0f;
	}

	SpriteActor::FrameMove(time, fElapsedTime);
}

void Fade::SetStatus(const EActorStatus status)
{
	// 変更を受け付けない
	return;
}

void Fade::FadeIn(float fadeTime)
{
	if (fadeTime <= 0.0f) fadeTime = 0.000001f;
	m_Status	  = ACTIVE;
	m_FadeTime	  = fadeTime;
	m_ChangeAlpha = 1.0f / (GAME->GetFPS() * fadeTime);
	m_Color		  = { 0.0f, 0.0f, 0.0f, 1.0f };
	m_Mode		  = EFadeMode::FADEIN;
}

void Fade::FadeOut(float fadeTime)
{
	if (fadeTime <= 0.0f) fadeTime = 0.000001f;
	m_Status	  = ACTIVE;
	m_FadeTime	  = fadeTime;
	m_ChangeAlpha = 1.0f / (GAME->GetFPS() * fadeTime);
	m_Color		  = { 0.0f, 0.0f, 0.0f, 0.0f };
	m_Mode		  = EFadeMode::FADEOUT;
}