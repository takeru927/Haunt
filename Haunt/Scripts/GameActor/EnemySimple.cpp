#include "EnemySimple.h"
#include "GameManager/Collision.h"
#include "GameMain.h"
#include "ApproximateShadow.h"

namespace {
	LPCWSTR FileName = L"Contents/Models/Ghost.obj";
}

EnemySimple::EnemySimple(Shader* shader)
	: ModelActor(L"Enemy", shader, FileName), m_Speed(0.0f)
{
	auto shadow = std::make_unique<ApproximateShadow>(this, 8.5f);
	this->AddChild(std::move(shadow));
}

HRESULT EnemySimple::CreateObject()
{
	m_Speed = 23.0f;

	XMVECTOR offset = { 0.0f,8.0f,0.0f,0.0f };
	this->AddSphereCollision(INFLICT, offset, 3.5f);

	if (FAILED(ModelActor::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void EnemySimple::FrameMove(const double& time, const float& fElapsedTime)
{
	XMVECTOR target = dynamic_cast<SceneActor*>(GAME->GetRootActor()->Search(L"Player"))->GetLocation();

	XMVECTOR moveVec = XMVector3Normalize(target - this->m_Location);
	this->m_Location += moveVec * (fElapsedTime * m_Speed);

	float cos = XMVectorGetX(XMVector3Dot({0.0f,0.0f,1.0f}, moveVec) / XMVector3Length(moveVec));
	float rot = acosf(cos);
	if (XMVectorGetX(XMVector3Dot({ 1.0f,0.0f,0.0f }, moveVec)) < 0.0f)
		rot = -rot;

	m_Rotation.m128_f32[1] = rot;

	ModelActor::FrameMove(time, fElapsedTime);
}

void EnemySimple::FrameRender(const double& time, const float& fElapsedTime)
{
	ModelActor::FrameRender(time, fElapsedTime);
}