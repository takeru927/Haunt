#include "TextWriter.h"
#include "DirectWrite.h"

TextWriter::TextWriter()
	: m_TextFormat(nullptr), m_SolidBrush(nullptr)
{
}

HRESULT TextWriter::SetTextFormat(float size, DirectX::XMFLOAT4 color)
{
	HRESULT hr = S_OK;

	// テキストフォントの設定
	hr = DWRITE->GetWriteFactory()->CreateTextFormat(
		L"メイリオ", 
		nullptr, 
		DWRITE_FONT_WEIGHT_NORMAL, 
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 
		size, 
		L"", 
		m_TextFormat.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
		return hr;

	// テキスト表示位置の設定
	hr = m_TextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	if (FAILED(hr))
		return hr;

	// テキストカラーの設定
	D2D1::ColorF colorF = { color.x, color.y, color.z, color.w };
	hr = DWRITE->GetRenderTarget()->CreateSolidColorBrush(colorF, m_SolidBrush.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void TextWriter::RenderText(LPCWSTR str, D2D1_RECT_F rect)
{
	auto renderTarget = DWRITE->GetRenderTarget();

	if (renderTarget)
	{
		renderTarget->BeginDraw();

		renderTarget->DrawTextW(
			str,
			static_cast<UINT32>(wcslen(str)),
			m_TextFormat.Get(),
			rect,
			m_SolidBrush.Get(),
			D2D1_DRAW_TEXT_OPTIONS_NONE
		);

		renderTarget->EndDraw();
	}
}

void TextWriter::SetColor(DirectX::XMFLOAT4 color)
{
	if (m_SolidBrush)
	{
		m_SolidBrush->SetColor({ color.x, color.y, color.z, color.w });
		m_SolidBrush->SetOpacity(color.w);
	}
}