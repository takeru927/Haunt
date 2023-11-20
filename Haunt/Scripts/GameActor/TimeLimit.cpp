#include "TimeLimit.h"
#include "GameMain.h"
#include "GameLibrary/DirectDevice.h"
#include <iomanip>

TimeLimit::TimeLimit()
	: TextActor(L"TimeLimit"), m_Time(0.0f)
{
}

HRESULT TimeLimit::CreateObject()
{
	HRESULT hr = S_OK;

	m_Time = 400.0f;

	// テキストの生成
	DirectX::XMFLOAT4 color = { 1.0f,1.0f,1.0f,1.0f };
	hr = TextActor::CreateObject(50.0f, color);
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void TimeLimit::FrameMove(const double& time, const float& fElapsedTime)
{
	m_Time -= fElapsedTime;

	// タイムアップ
	if (m_Time <= 0.0f) {
		GAME->GetRootActor()->Search(L"MainScene")->SetStatus(DEAD);
		GAME->OpenScene(EScene::GAMEOVER);
	}

	// 残り時間が少なくなったら文字色を変更
	if (m_Time <= 100.0f)
		m_Color = { 1.0f,0.2f,0.2f,1.0f };

	std::wostringstream text;
	text << std::fixed << std::setprecision(2) << m_Time;
	wcscpy_s(m_Text, text.str().c_str());

	TextActor::FrameMove(time, fElapsedTime);
}

void TimeLimit::FrameRender(const double& time, const float& fElapsedTime)
{
	TextActor::FrameRender(time, fElapsedTime);
}

void TimeLimit::OnResizedSwapChain()
{
	auto viewport = D3D->GetScreenViewport();
	m_Rect = D2D1::RectF(0, 25, viewport.Width, 100);
}