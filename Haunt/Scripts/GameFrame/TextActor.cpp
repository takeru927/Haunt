#include "TextActor.h"

TextActor::TextActor(LPCWSTR name)
	: Actor(name), m_Writer(nullptr), m_Rect({ 0.0f,0.0f,0.0f,0.0f }), m_Color({ 1.0f,1.0f,1.0f,1.0f })
{
	wcscpy_s(m_Text, L"");
}

HRESULT TextActor::CreateObject(float size, DirectX::XMFLOAT4 color)
{
	HRESULT hr = S_OK;

	m_Color = color;

	// テキストの設定
	m_Writer = std::make_unique<TextWriter>();
	m_Writer->SetTextFormat(size, color);
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void TextActor::FrameMove(const double& time, const float& fElapsedTime)
{
	Actor::FrameMove(time, fElapsedTime);
}

void TextActor::FrameRender(const double& time, const float& fElapsedTime)
{
	m_Writer->SetColor(m_Color);
	m_Writer->RenderText(m_Text, m_Rect);

	Actor::FrameRender(time, fElapsedTime);
}

void TextActor::SetText(LPCWSTR text)
{
	wcscpy_s(m_Text, text);
}

void TextActor::SetRect(const D2D1_RECT_F rect)
{
	m_Rect = rect;
}