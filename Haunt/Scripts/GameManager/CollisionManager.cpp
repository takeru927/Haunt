#include "CollisionManager.h"
#include "CollisionCaluclater.h"

using namespace CollisionCaluclater;

// 形状別のコリジョン計算表
const std::function<bool(Collision* const, Collision* const, XMVECTOR*)>
CollisionManager::m_CheckCollide[SHAPENUM][SHAPENUM] = {
	{SphereAndSphere , SphereAndCapsule , SphereAndBox },
	{CapsuleAndSphere, CapsuleAndCapsule, CapsuleAndBox},
	{BoxAndSphere	 , BoxAndCapsule	, BoxAndBox	   },
};

// タイプ別のコリジョン対応表
const CollisionResponse CollisionManager::m_CollisionMatrix[TYPENUM][TYPENUM] = {
	// DEFAULT, CHARACTER, INFLICT, OBJECT
	{ BLOCK,  BLOCK,	 BLOCK,	   BLOCK,   }, // DEFAULT
	{ BLOCK,  BLOCK,	 OVERLAP,  BLOCK,   }, // CHARACTER
	{ BLOCK,  OVERLAP,   NOTHING,  NOTHING, }, // INFLICT
	{ BLOCK,  BLOCK,	 NOTHING,  NOTHING, }, // OBJECT
};

CollisionManager::CollisionManager()
{
	
}

void CollisionManager::Update()
{
	for (auto& collision : m_Pool)
		collision->Update();
}

void CollisionManager::SetCollision(Collision* collision)
{
	m_Pool.push_back(collision);
}

void CollisionManager::ReleaseCollision(Collision* const target)
{
	for (auto it = m_Pool.begin(); it != m_Pool.end(); it++) {
		if ((*it) == target) {
			m_Pool.erase(it);
			return;
		}
	}
}

void CollisionManager::CalculateCollision()
{
	// 総当たりで判定を行う
	for (auto it = m_Pool.begin(); it != m_Pool.end(); it++) {
		if ((*it)->GetActor()->GetStatus() == NOUSE)
			continue;

		for (auto other = std::next(it); other != m_Pool.end(); other++) {
			if ((*other)->GetActor()->GetStatus() == NOUSE ||
				m_CollisionMatrix[(*it)->GetType()][(*other)->GetType()] == NOTHING)
				continue;

			bool hit = false;
			// オーバーラップ
			if (m_CollisionMatrix[(*it)->GetType()][(*other)->GetType()] == OVERLAP)
				hit = m_CheckCollide[(*it)->GetShape()][(*other)->GetShape()]((*it), (*other), nullptr);

			// ブロック
			else if (m_CollisionMatrix[(*it)->GetType()][(*other)->GetType()] == BLOCK) {
				XMVECTOR* pushbackVec = new XMVECTOR({0.0f});
				hit = m_CheckCollide[(*it)->GetShape()][(*other)->GetShape()]((*it), (*other), pushbackVec);

				PushbackActor((*it), (*other), *pushbackVec);

				delete pushbackVec;
			}

			if (hit) {
				(*it)->GetActor()->OnCollide((*other)->GetActor());
				(*other)->GetActor()->OnCollide((*it)->GetActor());
			}
		}
	}
}

// コリジョンによるアクターの押し戻し
void CollisionManager::PushbackActor(Collision* collision1, Collision* collision2, XMVECTOR pushbackVec)
{
	if (collision1->GetType() == CHARACTER && collision2->GetType() != CHARACTER)
	{
		XMVECTOR afterLocation = collision1->GetActor()->GetLocation() + pushbackVec;
		collision1->GetActor()->SetLocation(afterLocation);
	}
	else if (collision1->GetType() != CHARACTER && collision2->GetType() == CHARACTER)
	{
		XMVECTOR afterLocation = collision2->GetActor()->GetLocation() - pushbackVec;
		collision2->GetActor()->SetLocation(afterLocation);
	}
}