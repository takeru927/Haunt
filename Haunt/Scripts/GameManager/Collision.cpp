#include "Collision.h"
#include "GameMain.h"
#include "GameScene/MainScene.h"

Collision::Collision(SceneActor* actor, CollisionShape shape, CollisionType type, XMVECTOR offset)
	: m_Actor(actor), m_Shape(shape), m_Type(type), m_Offset(offset), m_Location({ 0.0f })
{
	GAME->GetCollisionManager()->SetCollision(this);
}

Collision::~Collision()
{
	GAME->GetCollisionManager()->ReleaseCollision(this);
}

void Collision::Update()
{
	XMMATRIX mat = XMMatrixRotationRollPitchYawFromVector(m_Actor->GetRotation());
	XMVECTOR offset = XMVector3Transform(m_Offset, mat);

	m_Location = m_Actor->GetLocation() + offset;
}

SceneActor* Collision::GetActor() const
{
	return m_Actor;
}

DirectX::XMVECTOR Collision::GetLocation() const
{
	return m_Location;
}

CollisionShape Collision::GetShape() const
{
	return m_Shape;
}

CollisionType Collision::GetType() const
{
	return m_Type;
}

XMVECTOR Collision::GetOffset() const
{
	return m_Offset;
}

//--------------------------------------------------------------------------------------
// 球コリジョン
//--------------------------------------------------------------------------------------

SphereCollision::SphereCollision(SceneActor* actor, CollisionType type, XMVECTOR offset, float radius, bool visible)
	: Collision(actor, SPHERE, type, offset), m_Radius(radius)
{
	if (visible)
	{
		// コリジョンの表示
		auto check = std::make_unique<CheckCollision>(this);
		check->CreateObject();
		check->CreateCapsuleVertex(radius, 0.0f);

		actor->AddChild(std::move(check));
	}
}

float SphereCollision::GetRadius() const
{
	return m_Radius;
}

//--------------------------------------------------------------------------------------
// カプセルコリジョン
//--------------------------------------------------------------------------------------

CapsuleCollision::CapsuleCollision(SceneActor* actor, CollisionType type, XMVECTOR offset, float radius, float height, bool visible)
	: Collision(actor, CAPSULE, type, offset)
	, m_Radius(radius), m_Height(height), m_StartLocation({ 0.0f }), m_EndLocation({ 0.0f })
{
	if (visible)
	{
		// コリジョンの表示
		auto check = std::make_unique<CheckCollision>(this);
		check->CreateObject();
		check->CreateCapsuleVertex(radius, height);

		actor->AddChild(std::move(check));
	}
}

void CapsuleCollision::Update()
{
	Collision::Update();

	XMVECTOR capsuleVec = { 0.0f, m_Height * 0.5f, 0.0f, 0.0f };
	XMMATRIX rotMat = XMMatrixRotationRollPitchYawFromVector(m_Actor->GetRotation());

	capsuleVec = XMVector3Transform(capsuleVec, rotMat);

	m_StartLocation = m_Location + capsuleVec;
	m_EndLocation = m_Location - capsuleVec;
}

float CapsuleCollision::GetRadius() const
{
	return m_Radius;
}

float CapsuleCollision::GetHeight() const
{
	return m_Height;
}

XMVECTOR CapsuleCollision::GetStart() const
{
	return m_StartLocation;
}

XMVECTOR CapsuleCollision::GetEnd() const
{
	return m_EndLocation;
}

//--------------------------------------------------------------------------------------
// ボックスコリジョン
//--------------------------------------------------------------------------------------

BoxCollision::BoxCollision(SceneActor* actor, CollisionType type, XMVECTOR offset, XMVECTOR extent, bool visible)
	: Collision(actor, BOX, type, offset)
	, m_Extent(extent), m_Axis{{ 0.0f },{ 0.0f },{ 0.0f }}
{
	if (visible)
	{
		// コリジョンの表示
		auto check = std::make_unique<CheckCollision>(this);
		check->CreateObject();
		check->CreateBoxVertex(extent);

		actor->AddChild(std::move(check));
	}
}

void BoxCollision::Update()
{
	Collision::Update();

	XMMATRIX rotMat = XMMatrixRotationRollPitchYawFromVector(m_Actor->GetRotation());
	XMVECTOR axis[3] = {
		{ 1.0f,0.0f,0.0f,0.0f },
		{ 0.0f,1.0f,0.0f,0.0f },
		{ 0.0f,0.0f,1.0f,0.0f }
	};

	for (int i = 0; i < 3; i++)
		m_Axis[i] = XMVector3Transform(axis[i], rotMat);
}

XMVECTOR BoxCollision::GetExtent()const
{ 
	return m_Extent; 
}

XMVECTOR BoxCollision::GetAxis(int index) const
{
	return m_Axis[index];
}