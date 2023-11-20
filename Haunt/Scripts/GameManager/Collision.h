#pragma once
#include "GameFrame/SceneActor.h"
#include "CheckCollision.h"

// コリジョンの形状
enum CollisionShape{
	SPHERE,		// 球
	CAPSULE,	// カプセル
	BOX,		// ボックス
};

// コリジョンのタイプ
enum CollisionType {
	DEFAULT,
	CHARACTER,
	INFLICT,
	OBJECT,
};

//--------------------------------------------------------------------------------------
// コリジョン基底クラス
//--------------------------------------------------------------------------------------
__declspec(align(16))
class Collision
{
protected:
	SceneActor*		m_Actor;
	CollisionShape	m_Shape;
	CollisionType	m_Type;
	XMVECTOR		m_Offset;

	XMVECTOR m_Location;

public:
	Collision(SceneActor* actor, CollisionShape shape, CollisionType type, XMVECTOR offset);
	virtual ~Collision();

	virtual void Update();

	SceneActor* GetActor() const;
	XMVECTOR GetLocation() const;
	CollisionShape GetShape() const;
	CollisionType GetType() const;
	XMVECTOR GetOffset() const;

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(Collision));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};

//--------------------------------------------------------------------------------------
// 球コリジョン
//--------------------------------------------------------------------------------------
class SphereCollision :public Collision
{
private:
	float m_Radius;

public:
	SphereCollision(SceneActor* actor, CollisionType type, XMVECTOR offset, float radius, bool visible = false);
	~SphereCollision() = default;

	float GetRadius() const;
};

//--------------------------------------------------------------------------------------
// カプセルコリジョン
//--------------------------------------------------------------------------------------
__declspec(align(16))
class CapsuleCollision : public Collision
{
private:
	float m_Radius;
	float m_Height;

	XMVECTOR m_StartLocation;
	XMVECTOR m_EndLocation;

public:
	CapsuleCollision(SceneActor* actor, CollisionType type, XMVECTOR offset, float radius, float height, bool visible = false);
	~CapsuleCollision() = default;

	void Update() override;

	float GetRadius() const;
	float GetHeight() const;
	XMVECTOR GetStart() const;
	XMVECTOR GetEnd() const;

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(CapsuleCollision));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};

//--------------------------------------------------------------------------------------
// ボックスコリジョン
//--------------------------------------------------------------------------------------
__declspec(align(16))
class BoxCollision : public Collision
{
private:
	XMVECTOR m_Extent;
	XMVECTOR m_Axis[3];

public:
	BoxCollision(SceneActor* actor, CollisionType type, XMVECTOR offset, XMVECTOR extent, bool visible = false);
	~BoxCollision() = default;

	void Update() override;

	XMVECTOR GetExtent() const;
	XMVECTOR GetAxis(int index) const;

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(BoxCollision));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};