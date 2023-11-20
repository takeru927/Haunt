#pragma once
#include "Collision.h"

//--------------------------------------------------------------------------------------
// ÉRÉäÉWÉáÉìåvéZä÷êîåQ
//--------------------------------------------------------------------------------------

namespace CollisionCaluclater
{
	bool SphereAndSphere(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool SphereAndCapsule(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool SphereAndBox(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool CapsuleAndSphere(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool CapsuleAndCapsule(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool CapsuleAndBox(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool BoxAndSphere(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool BoxAndCapsule(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
	bool BoxAndBox(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec);
};