#pragma once
#include "Actor.h"
#include "GameLibrary/TextWriter.h"

class TextActor : public Actor
{
protected:
	std::unique_ptr<TextWriter> m_Writer;

	WCHAR			  m_Text[1024];
	D2D1_RECT_F		  m_Rect;
	DirectX::XMFLOAT4 m_Color;

public:
	TextActor(LPCWSTR name);
	virtual ~TextActor() = default;

	HRESULT CreateObject(float size, DirectX::XMFLOAT4 color);
	virtual void FrameMove(const double& time, const float& fElapsedTime);
	virtual void FrameRender(const double& time, const float& fElapsedTime);

	void SetText(LPCWSTR text);
	void SetRect(const D2D1_RECT_F rect);
};