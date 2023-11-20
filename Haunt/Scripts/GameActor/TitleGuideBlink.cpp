#include "TitleGuideBlink.h"
#include "GameLibrary/DirectDevice.h"
#include "GameMain.h"

TitleGuideBlink::TitleGuideBlink(const float interval, const float blinkTime)
	: TextActor(L"TitleGuideBlink"), m_Interval(interval), m_BlinkTime(blinkTime), m_Timer(interval)
	, m_BlinkStatus(EBlinkState::VISIBLE)
{
	wcscpy_s(m_Text, L"Press Spece to Start");

	m_ChangeAlpha = 1.0f / (GAME->GetFPS() * blinkTime);
}

HRESULT TitleGuideBlink::CreateObject()
{
	HRESULT hr = S_OK;

	// テキストの生成
	XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	hr = TextActor::CreateObject(45.0f, color);
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void TitleGuideBlink::FrameMove(const double& time, const float& fElapsedTime)
{
	switch (m_BlinkStatus)
	{
	case EBlinkState::BLINKOUT:

		// 半分で点滅折り返し
		if (m_Timer <= 0.0f)
		{
			m_BlinkStatus = EBlinkState::BLINKIN;
			m_Timer = m_BlinkTime;
		}

		m_Color.w -= m_ChangeAlpha;
		if (m_Color.w <= 0.0f)
			m_Color.w = 0.0f;

		break;

	case EBlinkState::BLINKIN:

		if (m_Timer <= 0.0f)
		{
			m_BlinkStatus = EBlinkState::VISIBLE;
			m_Timer = m_Interval;
			m_Color.w = 1.0f;
		}

		m_Color.w += m_ChangeAlpha;
		if (m_Color.w >= 1.0f)
			m_Color.w = 1.0f;

		break;

	case EBlinkState::VISIBLE:

		if (m_Timer <= 0.0f)
		{
			m_BlinkStatus = EBlinkState::BLINKOUT;
			m_Timer = m_BlinkTime;
		}

		break;
	}
	
	m_Timer -= fElapsedTime;

	TextActor::FrameMove(time, fElapsedTime);
}

void TitleGuideBlink::FrameRender(const double& time, const float& fElapsedTime)
{
	TextActor::FrameRender(time, fElapsedTime);
}

void TitleGuideBlink::OnResizedSwapChain()
{
	auto viewport = D3D->GetScreenViewport();
	m_Rect = D2D1::RectF(0, viewport.Height - 250, viewport.Width, viewport.Height);
}