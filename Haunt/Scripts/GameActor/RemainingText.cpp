#include "RemainingText.h"
#include "GameMain.h"
#include "GameLibrary/DirectDevice.h"

RemainingText::RemainingText()
	: TextActor(L"RemainingText")
{
}

HRESULT RemainingText::CreateObject()
{
	HRESULT hr = S_OK;

	// �e�L�X�g�̐���
	DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	hr = TextActor::CreateObject(40.0f, color);
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void RemainingText::FrameMove(const double& time, const float& fElapsedTime)
{
	// �c��R�C�����̎擾
	size_t count = GAME->GetRootActor()->Search(L"CoinManager")->GetChildren()->size();

	std::wostringstream text;
	if (count <= 0) {
		m_Color = { 1.0f,1.0f,0.0f,1.0f };
		text << L"�o���֌�����";
	}
	else
		text << L"�c��R�C���F" << count;

	wcscpy_s(m_Text, text.str().c_str());

	TextActor::FrameMove(time, fElapsedTime);
}

void RemainingText::FrameRender(const double& time, const float& fElapsedTime)
{
	TextActor::FrameRender(time, fElapsedTime);
}

void RemainingText::OnResizedSwapChain()
{
	auto viewport = D3D->GetScreenViewport();
	m_Rect = D2D1::RectF(0, 90, viewport.Width, 140);
}