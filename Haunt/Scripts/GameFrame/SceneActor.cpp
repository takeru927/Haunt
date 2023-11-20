#include "SceneActor.h"
#include "GameManager/Collision.h"
#include "GameManager/MeshManager.h"

using namespace DirectX;

SceneActor::SceneActor(LPCWSTR name)
	: Actor(name), m_World(XMMatrixIdentity())
	, m_Location({ 0.0f }), m_Rotation({ 0.0f }), m_Scale({ 1.0f, 1.0f, 1.0f, 1.0f })
{
}

SceneActor::~SceneActor()
{
	for (auto it = m_Collision.begin(); it != m_Collision.end();) {
		(*it).reset();
		it = m_Collision.erase(it);
	}
}

HRESULT SceneActor::CreateObject()
{
	auto scale = XMMatrixScalingFromVector(m_Scale);
	auto rotation = XMMatrixRotationRollPitchYawFromVector(m_Rotation);
	auto location = XMMatrixTranslationFromVector(m_Location);
	m_World = scale * rotation * location;

	if (FAILED(Actor::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void SceneActor::OnCollide(SceneActor* other)
{
}

void SceneActor::AddSphereCollision(CollisionType type, XMVECTOR offset, float radius, bool visible)
{
	auto sphere = std::make_unique<SphereCollision>(this, type, offset, radius, visible);
	m_Collision.emplace_back(std::move(sphere));
}

void SceneActor::AddCapsuleCollision(CollisionType type, XMVECTOR offset, float radius, float height, bool visible)
{
	auto capsule = std::make_unique<CapsuleCollision>(this, type, offset, radius, height, visible);
	m_Collision.emplace_back(std::move(capsule));
}

void SceneActor::AddBoxCollision(CollisionType type, XMVECTOR offset, XMVECTOR extent, bool visible)
{
	auto box = std::make_unique<BoxCollision>(this, type, offset, extent, visible);
	m_Collision.emplace_back(std::move(box));
}

XMMATRIX SceneActor::GetWorld() const 
{
	return m_World; 
}

XMVECTOR SceneActor::GetLocation() const 
{ 
	return m_Location; 
}

XMVECTOR SceneActor::GetRotation() const 
{ 
	return m_Rotation; 
}

Collision* SceneActor::GetCollision() const 
{ 
	return m_Collision.begin()->get(); 
}

void SceneActor::SetWorld(const XMMATRIX world) 
{
	m_World = world; 
}

void SceneActor::SetLocation(const XMVECTOR location) 
{
	m_Location = location; 
}

void SceneActor::SetRotation(const XMVECTOR rotation) 
{
	m_Rotation = rotation; 
}

void SceneActor::SetScale(const XMVECTOR scale) 
{
	m_Scale = scale; 
}