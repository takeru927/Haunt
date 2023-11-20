#pragma once
#include "Collision.h"

enum CollisionResponse {
	NOTHING,
	OVERLAP,
	BLOCK,
};

constexpr int TYPENUM = 4;
constexpr int SHAPENUM = 3;

class CollisionManager
{
private:
	std::list<Collision*> m_Pool;

	static const std::function<bool(Collision* const, Collision* const, XMVECTOR*)> m_CheckCollide[SHAPENUM][SHAPENUM];
	static const CollisionResponse m_CollisionMatrix[TYPENUM][TYPENUM];

public:
	CollisionManager();
	~CollisionManager() = default;

	void Update();

	void SetCollision(Collision* collision);
	void ReleaseCollision(Collision* const target);
	void CalculateCollision();

private:
	void PushbackActor(Collision* collision1, Collision* collision2, XMVECTOR pushbackVec);
};