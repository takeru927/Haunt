#include "CollisionCaluclater.h"

//--------------------------------------------------------------------------------------
// �v�Z�̕⏕�֐�
//--------------------------------------------------------------------------------------
namespace
{
	// �_�ƒ����̍ŒZ���������߁A�x�N�g���W�����Q�ƂŕԂ�
	float CalcPointLineDist(const XMVECTOR& point, const XMVECTOR& line, const XMVECTOR& lineStart, float& ratio)
	{
		float lineLenSq = XMVectorGetX(XMVector3Dot(line, line));
		ratio = XMVectorGetX(XMVector3Dot(line, point - lineStart)) / lineLenSq;

		XMVECTOR linePoint = line * ratio + lineStart;
		return XMVectorGetX(XMVector3Length(linePoint - point));
	}

	// 2�̃{�b�N�X�𕪂��镽��(����������)�����݂��邩���v�Z����
	bool IsSeparatingPlane(const XMVECTOR& sepAxis, const XMVECTOR& plane, const XMVECTOR(&box1)[3], const XMVECTOR(&box2)[3])
	{
		float distance = fabs(XMVectorGetX(XMVector3Dot(sepAxis, plane)));

		float boxLen1 = fabs(XMVectorGetX(XMVector3Dot(box1[0], plane)))
					  + fabs(XMVectorGetX(XMVector3Dot(box1[1], plane)))
					  + fabs(XMVectorGetX(XMVector3Dot(box1[2], plane)));

		float boxLen2 = fabs(XMVectorGetX(XMVector3Dot(box2[0], plane)))
					  + fabs(XMVectorGetX(XMVector3Dot(box2[1], plane)))
					  + fabs(XMVectorGetX(XMVector3Dot(box2[2], plane)));

		return distance > boxLen1 + boxLen2;
	}
}

//--------------------------------------------------------------------------------------
// �� vs ��
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::SphereAndSphere(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	auto sphere1 = dynamic_cast<SphereCollision*>(actor1);
	auto sphere2 = dynamic_cast<SphereCollision*>(actor2);

	double distance = XMVectorGetX(XMVector3Length(sphere1->GetLocation() - sphere2->GetLocation()));

	if (distance <= sphere1->GetRadius() + sphere2->GetRadius()) {
		// �����߂��x�N�g���̎Z�o
		if (pushVec != nullptr)
		{
			XMVECTOR radiusVec1 = XMVector3Normalize(sphere2->GetLocation() - sphere1->GetLocation()) * sphere1->GetRadius();
			XMVECTOR radiusVec2 = XMVector3Normalize(sphere1->GetLocation() - sphere2->GetLocation()) * sphere2->GetRadius();

			*pushVec = radiusVec2 - (sphere1->GetLocation() - sphere2->GetLocation() + radiusVec1);
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �� vs �J�v�Z��
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::SphereAndCapsule(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	SphereCollision* sphere = nullptr;
	CapsuleCollision* capsule = nullptr;

	// �Ή������R���W�����Ƀ_�E���L���X�g����
	if (actor1->GetShape() == SPHERE) {
		sphere = dynamic_cast<SphereCollision*>(actor1);
		capsule = dynamic_cast<CapsuleCollision*>(actor2);
	}
	else {
		sphere = dynamic_cast<SphereCollision*>(actor2);
		capsule = dynamic_cast<CapsuleCollision*>(actor1);
	}

	XMVECTOR A = capsule->GetStart();
	XMVECTOR B = capsule->GetEnd();
	XMVECTOR P = sphere->GetLocation();

	// �v�Z�p�̊e�x�N�g�����擾
	XMVECTOR AB = B - A;
	XMVECTOR BA = A - B;
	XMVECTOR AP = P - A;
	XMVECTOR BP = P - B;

	BYTE pointCase;

	// �ꍇ�������āA�_�Ɛ����̍ŒZ���������߂�
	float distance;
	// �_��A���O���ɂ���
	if (XMVectorGetX(XMVector3Dot(AB, AP)) < 0.0f) {
		distance = XMVectorGetX(XMVector3Length(AP));
		pointCase = 0;
	}
	// �_��B���O���ɂ���
	else if (XMVectorGetX(XMVector3Dot(BA, BP)) < 0.0f) {
		distance = XMVectorGetX(XMVector3Length(BP));
		pointCase = 1;
	}
	// �_��������ɂ���
	else {
		float cross = (XMVectorGetX(XMVector3Length(XMVector3Cross(AB, AP))));
		distance = cross / capsule->GetHeight();
		pointCase = 2;
	}

	// �ڐG���Ă���΁Atrue��Ԃ�
	if (distance <= sphere->GetRadius() + capsule->GetRadius()) {
		// �����߂��x�N�g���̎Z�o
		if (pushVec != nullptr)
		{
			// �_��A���O���ɂ���
			if (pointCase == 0) {
				XMVECTOR capsuleRadiusVec = XMVector3Normalize(AP) * capsule->GetRadius();
				XMVECTOR sphereRadiusVec = XMVector3Normalize(A - P) * sphere->GetRadius();

				*pushVec = capsuleRadiusVec - (AP + sphereRadiusVec);
			}
			// �_��B���O���ɂ���
			else if (pointCase == 1) {
				XMVECTOR capsuleRadiusVec = XMVector3Normalize(BP) * capsule->GetRadius();
				XMVECTOR sphereRadiusVec = XMVector3Normalize(B - P) * sphere->GetRadius();

				*pushVec = capsuleRadiusVec - (BP + sphereRadiusVec);
			}
			// �_��������ɂ���
			else {
				XMVECTOR AtoCapsulePoint = XMVector3Normalize(AB) * XMVectorGetX(XMVector3Dot(AB, XMVector3Normalize(AP)));
				XMVECTOR capsulePoint = A + AtoCapsulePoint;
				XMVECTOR CapsulePointToP = P - capsulePoint;
				XMVECTOR capsuleRadiusVec = XMVector3Normalize(CapsulePointToP) * capsule->GetRadius();
				XMVECTOR sphereRadiusVec = XMVector3Normalize(capsulePoint - P) * sphere->GetRadius();

				*pushVec = capsuleRadiusVec - (CapsulePointToP + sphereRadiusVec);
			}
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �� vs �{�b�N�X
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::SphereAndBox(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	SphereCollision* sphere = nullptr;
	BoxCollision* box = nullptr;

	// �Ή������R���W�����Ƀ_�E���L���X�g����
	if (actor1->GetShape() == SPHERE) {
		sphere = dynamic_cast<SphereCollision*>(actor1);
		box = dynamic_cast<BoxCollision*>(actor2);
	}
	else {
		sphere = dynamic_cast<SphereCollision*>(actor2);
		box = dynamic_cast<BoxCollision*>(actor1);
	}

	XMVECTOR boxSphereVec = sphere->GetLocation() - box->GetLocation();

	// �e���̂͂ݏo���x�N�g�������߂�
	XMVECTOR stickOutVec = { 0.0f };
	for (int i = 0; i < 3; i++)
	{
		float axisLen = box->GetExtent().m128_f32[i];
		if (axisLen <= 0) continue;

		// �_�Ƃ̈ʒu�֌W���ꍇ��������l
		float s = XMVectorGetX(XMVector3Dot(boxSphereVec, box->GetAxis(i))) / axisLen;

		if (s > 1.0f)
			stickOutVec += (s - 1) * axisLen * box->GetAxis(i);
		else if (s < -1.0f)
			stickOutVec += (s + 1) * axisLen * box->GetAxis(i);
	}

	float distance = XMVectorGetX(XMVector3Length(stickOutVec));
	if (distance <= sphere->GetRadius()) {
		// �����߂��x�N�g���̎Z�o
		if (pushVec != nullptr)
		{
			// ���̒��S���{�b�N�X�ɓ��荞��ł���
			if (distance == 0.0f) {
				int minAxisIndex;
				float dots[3] = {};
				float minLength = 10000.0f;
				for (int i = 0; i < 3; i++)
				{
					dots[i] = XMVectorGetX(XMVector3Dot(boxSphereVec, box->GetAxis(i)));
					float length = box->GetExtent().m128_f32[i] - fabs(dots[i]);
					if (minLength > length) {
						minLength = length;
						minAxisIndex = i;
					}
				}

				if (dots[minAxisIndex] > 0.0f) {
					*pushVec = +box->GetAxis(minAxisIndex) * (sphere->GetRadius() + minLength);
				}
				else {
					*pushVec = -box->GetAxis(minAxisIndex) * (sphere->GetRadius() + minLength);
				}
			}

			else
				*pushVec = XMVector3Normalize(stickOutVec) * (sphere->GetRadius() - distance);
		}
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �J�v�Z�� vs ��
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::CapsuleAndSphere(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	if (SphereAndCapsule(actor2, actor1, pushVec)) {
		if (pushVec != nullptr)
			*pushVec = -*pushVec;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �J�v�Z�� vs �J�v�Z��
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::CapsuleAndCapsule(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	auto capsule1 = dynamic_cast<CapsuleCollision*>(actor1);
	auto capsule2 = dynamic_cast<CapsuleCollision*>(actor2);

	bool endCalc = false;
	float distance = 0.0f;
	float capPointRatio1 = 0.0f;
	float capPointRatio2 = 0.0f;

	XMVECTOR capsuleVec1 = capsule1->GetEnd() - capsule1->GetStart();
	XMVECTOR capsuleVec2 = capsule2->GetEnd() - capsule2->GetStart();

	// �����͕��s���H
	if (XMVectorGetX(XMVector3Length(XMVector3Cross(capsuleVec1, capsuleVec2))) == 0.0f)
	{
		distance = CalcPointLineDist(capsule2->GetStart(), capsuleVec1, capsule1->GetStart(), capPointRatio1);

		if (0.0f <= capPointRatio1 && capPointRatio1 <= 1.0f)
			endCalc = true;
	}
	else
	{
		float cap12 = XMVectorGetX(XMVector3Dot(capsuleVec1, capsuleVec2));
		float capLenSq1 = XMVectorGetX(XMVector3Dot(capsuleVec1, capsuleVec1));
		float capLenSq2 = XMVectorGetX(XMVector3Dot(capsuleVec2, capsuleVec2));
		XMVECTOR startStart = capsule1->GetStart() - capsule2->GetStart();
		capPointRatio1 = (cap12 * XMVectorGetX(XMVector3Dot(capsuleVec2, startStart))
				  - capLenSq2 * XMVectorGetX(XMVector3Dot(capsuleVec1, startStart))) / (capLenSq1 * capLenSq2 - cap12 * cap12);
		
		XMVECTOR capPoint1 = capsuleVec1 * capPointRatio1 + capsule1->GetStart();
		capPointRatio2 = XMVectorGetX(XMVector3Dot(capsuleVec2, capPoint1 - capsule2->GetStart()));
		
		if (0.0f <= capPointRatio1 && capPointRatio1 <= 1.0f &&
			0.0f <= capPointRatio2 && capPointRatio2 <= 1.0f) {

			XMVECTOR capPoint2 = capsuleVec2 * capPointRatio2 + capsule2->GetStart();
			distance = XMVectorGetX(XMVector3Length(capPoint2 - capPoint1));

			endCalc = true;
		}
	}

	if (endCalc == false)
	{
		if		(capPointRatio1 < 0.0f) capPointRatio1 = 0.0f;
		else if (capPointRatio1 > 1.0f) capPointRatio1 = 1.0f;

		XMVECTOR capPoint1 = capsuleVec1 * capPointRatio1 + capsule1->GetStart();
		distance = CalcPointLineDist(capPoint1, capsuleVec2, capsule2->GetStart(), capPointRatio2);

		if (capPointRatio2 < 0.0f || capPointRatio2 > 1.0f)
		{
			if		(capPointRatio2 < 0.0f) capPointRatio2 = 0.0f;
			else if (capPointRatio2 > 1.0f) capPointRatio2 = 1.0f;

			XMVECTOR capPoint2 = capsuleVec2 * capPointRatio2 + capsule2->GetStart();
			distance = CalcPointLineDist(capPoint2, capsuleVec1, capsule1->GetStart(), capPointRatio1);

			if (capPointRatio1 < 0.0f || capPointRatio1 > 1.0f) 
			{
				if		(capPointRatio1 < 0.0f) capPointRatio1 = 0.0f;
				else if (capPointRatio1 > 1.0f) capPointRatio1 = 1.0f;

				capPoint1 = capsuleVec1 * capPointRatio1 + capsule1->GetStart();
				distance = XMVectorGetX(XMVector3Length(capPoint2 - capPoint1));
			}
		}
	}

	if (distance <= capsule1->GetRadius() + capsule2->GetRadius())
	{
		// �����߂��x�N�g���̎Z�o
		if (pushVec != nullptr)
		{
			XMVECTOR capPoint1 = capsuleVec1 * capPointRatio1 + capsule1->GetStart();
			XMVECTOR capPoint2 = capsuleVec2 * capPointRatio2 + capsule2->GetStart();

			XMVECTOR radiusVec1 = XMVector3Normalize(capPoint2 - capPoint1) * capsule1->GetRadius();
			XMVECTOR radiusVec2 = XMVector3Normalize(capPoint1 - capPoint2) * capsule2->GetRadius();

			*pushVec = radiusVec2 - (capPoint1 - capPoint2 + radiusVec1);
		}

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �J�v�Z�� vs �{�b�N�X
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::CapsuleAndBox(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	CapsuleCollision* capsule = nullptr;
	BoxCollision* box = nullptr;

	// �Ή������R���W�����Ƀ_�E���L���X�g����
	if (actor1->GetShape() == CAPSULE) {
		capsule = dynamic_cast<CapsuleCollision*>(actor1);
		box = dynamic_cast<BoxCollision*>(actor2);
	}
	else {
		capsule = dynamic_cast<CapsuleCollision*>(actor2);
		box = dynamic_cast<BoxCollision*>(actor1);
	}

	XMMATRIX rotMat = XMMatrixRotationRollPitchYawFromVector(box->GetActor()->GetRotation());
	XMMATRIX rotMatInv = XMMatrixInverse(nullptr, rotMat);

	// �{�b�N�X�̎��ɍ��킹��
	XMVECTOR boxCapStart = XMVector3Transform(capsule->GetStart() - box->GetLocation(), rotMat) + box->GetLocation();
	XMVECTOR boxCapEnd = XMVector3Transform(capsule->GetEnd() - box->GetLocation(), rotMat) + box->GetLocation();

	XMVECTOR capAxis = boxCapEnd - boxCapStart;
	float capLengthSq = XMVectorGetX(XMVector3Dot(capAxis, capAxis));
	float pointRatio = XMVectorGetX(XMVector3Dot(capAxis, box->GetLocation() - boxCapStart)) / capLengthSq;
	if (pointRatio > 1.0f) pointRatio = 1.0f;
	else if (pointRatio < 0.0f) pointRatio = 0.0f;

	XMVECTOR closestPoint = capAxis * pointRatio + capsule->GetStart();

	//----------------------------------------------------
	// ���Ƃ́A�� vs �{�b�N�X�Ƃ��Čv�Z���Ă���

	XMVECTOR boxSphereVec = closestPoint - box->GetLocation();

	// �e���̂͂ݏo���x�N�g�������߂�
	XMVECTOR stickOutVec = { 0.0f };
	for (int i = 0; i < 3; i++)
	{
		float axisLen = box->GetExtent().m128_f32[i];
		if (axisLen <= 0) continue;

		// �_�Ƃ̈ʒu�֌W���ꍇ��������l
		float s = XMVectorGetX(XMVector3Dot(boxSphereVec, box->GetAxis(i))) / axisLen;

		if (s > 1.0f)
			stickOutVec += (s - 1) * axisLen * box->GetAxis(i);
		else if (s < -1.0f)
			stickOutVec += (s + 1) * axisLen * box->GetAxis(i);
	}

	float distance = XMVectorGetX(XMVector3Length(stickOutVec));
	if (distance <= capsule->GetRadius()) {
		// �����߂��x�N�g���̎Z�o
		if (pushVec != nullptr)
		{
			// ���̒��S���{�b�N�X�ɓ��荞��ł���
			if (distance == 0.0f) {
				int minAxisIndex;
				float dots[3] = {};
				float minLength = 10000.0f;
				for (int i = 0; i < 3; i++)
				{
					dots[i] = XMVectorGetX(XMVector3Dot(boxSphereVec, box->GetAxis(i)));
					float length = box->GetExtent().m128_f32[i] - fabs(dots[i]);
					if (minLength > length) {
						minLength = length;
						minAxisIndex = i;
					}
				}

				if (dots[minAxisIndex] > 0.0f) {
					*pushVec = +box->GetAxis(minAxisIndex) * (capsule->GetRadius() + minLength);
				}
				else {
					*pushVec = -box->GetAxis(minAxisIndex) * (capsule->GetRadius() + minLength);
				}
			}

			else
				*pushVec = XMVector3Normalize(stickOutVec) * (capsule->GetRadius() - distance);
		}
		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �{�b�N�X vs ��
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::BoxAndSphere(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	if (SphereAndBox(actor2, actor1, pushVec)) {
		if (pushVec != nullptr)
			*pushVec = -*pushVec;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �{�b�N�X vs �J�v�Z��
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::BoxAndCapsule(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	if (CapsuleAndBox(actor2, actor1, pushVec)) {
		if (pushVec != nullptr)
			*pushVec = -*pushVec;

		return true;
	}

	return false;
}

//--------------------------------------------------------------------------------------
// �{�b�N�X vs �{�b�N�X
//--------------------------------------------------------------------------------------
bool CollisionCaluclater::BoxAndBox(Collision* const actor1, Collision* const actor2, XMVECTOR* pushVec)
{
	auto box1 = dynamic_cast<BoxCollision*>(actor1);
	auto box2 = dynamic_cast<BoxCollision*>(actor2);

	// �e�{�b�N�X�̎�
	XMVECTOR axisNormal1[3] = { box1->GetAxis(0), box1->GetAxis(1), box1->GetAxis(2) };
	XMVECTOR axisNormal2[3] = { box2->GetAxis(0), box2->GetAxis(1), box2->GetAxis(2) };

	// �e�{�b�N�X�̎��̒���
	XMVECTOR boxVec1[3] = {
		axisNormal1[0] * box1->GetExtent().m128_f32[0],
		axisNormal1[1] * box1->GetExtent().m128_f32[1],
		axisNormal1[2] * box1->GetExtent().m128_f32[2]
	};
	XMVECTOR boxVec2[3] = {
		axisNormal2[0] * box2->GetExtent().m128_f32[0],
		axisNormal2[1] * box2->GetExtent().m128_f32[1],
		axisNormal2[2] * box2->GetExtent().m128_f32[2]
	};

	XMVECTOR separatingAxis = box2->GetLocation() - box1->GetLocation();

	// ���������ʂ����݂��鎞�_�ŁA�Փ˂��Ă��Ȃ�
	if (
		IsSeparatingPlane(separatingAxis, axisNormal1[0], boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, axisNormal1[1], boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, axisNormal1[2], boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, axisNormal2[0], boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, axisNormal2[1], boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, axisNormal2[2], boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[0], axisNormal2[0]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[0], axisNormal2[1]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[0], axisNormal2[2]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[1], axisNormal2[0]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[1], axisNormal2[1]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[1], axisNormal2[2]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[2], axisNormal2[0]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[2], axisNormal2[1]), boxVec1, boxVec2) ||
		IsSeparatingPlane(separatingAxis, XMVector3Cross(axisNormal1[2], axisNormal2[2]), boxVec1, boxVec2))
	{
		return false;
	}

	// �����߂��x�N�g���̎Z�o
	if (pushVec != nullptr)
	{
		// box2����Ƃ��āAbox1�Ƃ̈ʒu�֌W�����߂�
		XMVECTOR stickOutVec = { 0.0f };
		for (int i = 0; i < 3; i++)
		{
			float axisLen = box1->GetExtent().m128_f32[i];
			if (axisLen <= 0) continue;

			// �_�Ƃ̈ʒu�֌W���ꍇ��������l
			float s = XMVectorGetX(XMVector3Dot(separatingAxis, box1->GetAxis(i))) / axisLen;

			if (s > 1.0f)
				stickOutVec += (s - 1) * axisLen * box1->GetAxis(i);
			else if (s < -1.0f)
				stickOutVec += (s + 1) * axisLen * box1->GetAxis(i);
		}

		// �����o�������̎Z�o
		XMVECTOR basisVec = XMVector3Normalize(stickOutVec);
		float boxLen1 = fabs(XMVectorGetX(XMVector3Dot(boxVec2[0], basisVec)))
					  + fabs(XMVectorGetX(XMVector3Dot(boxVec2[1], basisVec)))
					  + fabs(XMVectorGetX(XMVector3Dot(boxVec2[2], basisVec)));
		float distance = fabs(boxLen1 - XMVectorGetX(XMVector3Length(stickOutVec)));

		*pushVec = -basisVec * distance;
	}

	return true;
}