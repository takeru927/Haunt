#pragma once
#include "GameFrame/AnimatedActor.h"

enum class EDirection { UP, UPRIGHT, RIGHT, DOWNRIGHT, DOWN, DOWNLEFT, LEFT, UPLEFT };
enum EAnimPlayer { IDLE, RUN,};

__declspec(align(16))
class Player : public AnimatedActor
{
private:
	float	   m_Speed;
	XMVECTOR   m_Velocity;
	EDirection m_Dir;
	bool	   m_isAir;
	bool	   m_isDead;

	XMVECTOR m_CameraLocation;		// カメラの位置
	XMFLOAT2 m_CameraRotation;		// カメラのX,Y回転
	float	 m_CameraDistance;	// カメラとプレイヤーの距離
	POINT	 m_MouseStart;
	POINT	 m_MouseMove;

public:
	Player(AnimationShader* shader);
	~Player() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnCollide(SceneActor* other) override;

	bool IsDead() const;

	void InitCamera(const float x, const float y, const float d);

	XMMATRIX GetCameraView() const;
	XMMATRIX GetCameraLocation() const;

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(Player));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}

private:
	void UpdateCamera();
	HRESULT LoadAnimation() override;
};