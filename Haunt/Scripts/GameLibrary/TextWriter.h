#pragma once
#include <d2d1.h>
#include <dwrite.h>

using Microsoft::WRL::ComPtr;

class TextWriter
{
private:
	ComPtr<IDWriteTextFormat>	 m_TextFormat;
	ComPtr<ID2D1SolidColorBrush> m_SolidBrush;

public:
	TextWriter();
	~TextWriter() = default;

	HRESULT SetTextFormat(float size, DirectX::XMFLOAT4 color);
	void RenderText(LPCWSTR str, D2D1_RECT_F rect);
	
	void SetColor(DirectX::XMFLOAT4 color);
};