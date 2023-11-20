#include "Coin.h"
#include "GameManager/XAudio2Manager.h"
#include "EffectManager.h"
#include "GameMain.h"
#include "ApproximateShadow.h"

namespace {
	LPCWSTR FileName = L"Contents/Models/Coin.obj";
}

Coin::Coin(Shader* shader)
	: ModelActor(L"Coin", shader, FileName)
{
	auto shadow = std::make_unique<ApproximateShadow>(this, 3.5f);
	this->AddChild(std::move(shadow));
}

HRESULT Coin::CreateObject()
{
	m_Scale = { 1.5f, 1.5f, 1.5f, 1.5f };

	XMVECTOR offset = { 0.0f,2.5f,0.0f,0.0f };
	this->AddSphereCollision(INFLICT, offset, 2.4f);

	if (FAILED(ModelActor::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void Coin::FrameMove(const double& time, const float& fElapsedTime)
{
	m_Rotation.m128_f32[1] += fElapsedTime * 5.0f;

	ModelActor::FrameMove(time, fElapsedTime);
}

void Coin::FrameRender(const double& time, const float& fElapsedTime)
{
	ModelActor::FrameRender(time, fElapsedTime);
}

void Coin::OnCollide(SceneActor* other)
{
	if (wcscmp(other->GetName(), L"Player") == 0)
	{
		// エフェクトの再生
		XMVECTOR location = m_Location;
		location.m128_f32[1] += 3.5f;
		dynamic_cast<EffectManager*>(GAME->GetRootActor()->Search(L"EffectManager"))
			->PlayEffect(location);

		// SEの再生
		XAudio2Manager::GetInstance()->Play(L"Coin");

		m_Status = DEAD;
	}
}