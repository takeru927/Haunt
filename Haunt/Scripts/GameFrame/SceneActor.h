#pragma once
#include "Actor.h"

class Collision;
enum CollisionType;

using namespace DirectX;

__declspec(align(16))
class SceneActor : public Actor
{
protected:
	std::list<std::unique_ptr<Collision>> m_Collision;

	XMMATRIX m_World;
	XMVECTOR m_Location;
	XMVECTOR m_Rotation;
	XMVECTOR m_Scale;

public:
	SceneActor(LPCWSTR name);
	virtual ~SceneActor();

	virtual HRESULT CreateObject();
	virtual void OnCollide(SceneActor* other);

	void AddSphereCollision(CollisionType type, XMVECTOR offset, float radius, bool visible = false);
	void AddCapsuleCollision(CollisionType type, XMVECTOR offset, float radius, float height, bool visible = false);
	void AddBoxCollision(CollisionType type, XMVECTOR offset, XMVECTOR extent, bool visible = false);

	XMMATRIX GetWorld() const;
	XMVECTOR GetLocation() const;
	XMVECTOR GetRotation() const;
	Collision* GetCollision() const;

	void SetWorld(const XMMATRIX world);
	void SetLocation(const XMVECTOR location);
	void SetRotation(const XMVECTOR rotation);
	void SetScale(const XMVECTOR scale);

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(SceneActor));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};