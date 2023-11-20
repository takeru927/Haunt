#include "Player.h"
#include "GameManager/InputManager.h"
#include "EffectManager.h"
#include "GameManager/XAudio2Manager.h"
#include "GameManager/CheckCollision.h"
#include "GameManager/Collision.h"
#include "ApproximateShadow.h"

namespace {
	LPCWSTR FileName = L"Contents/Models/Remy.fbx";

	const float Gravity = 4.0f;
}

//---------------------------------------------------------------------------------
Player::Player(AnimationShader* shader)
	: AnimatedActor(L"Player", shader, FileName)
	, m_Speed(0.0f), m_Velocity({ 0.0f }), m_Dir(EDirection::LEFT)
	, m_isAir(false), m_isDead(false)
	, m_CameraLocation({ 0.0f }), m_CameraRotation{ 0.0f,0.0f }, m_CameraDistance(0.0f)
	, m_MouseStart({ 0 }), m_MouseMove({ 0 })
{
	auto shadow = std::make_unique<ApproximateShadow>(this, 3.0f);
	this->AddChild(std::move(shadow));

	XAudio2Manager::GetInstance()->OpenWave(L"Dead", L"Contents/Sounds/GameOver.wav");
}

HRESULT Player::CreateObject()
{
	m_Speed = 35.0f;

	XMVECTOR offset = { 0.0f,3.8f,0.0f,0.0f };
	this->AddCapsuleCollision(CHARACTER, offset, 1.0f, 5.5f);

	if (FAILED(AnimatedActor::CreateObject()))
		return E_FAIL;
	
	return S_OK;
}

void Player::FrameMove(const double& time, const float& fElapsedTime)
{
	auto input = InputManager::GetInstance();

	// 移動入力を４ビットで表現する
	BYTE move = (input->GetKeyState("Up")    << 3)
			  + (input->GetKeyState("Right") << 2)
			  + (input->GetKeyState("Down")  << 1)
			  + (input->GetKeyState("Left")  << 0);

	// 移動処理
	if (move == 0x08) {			// up 1000
		m_Velocity.m128_f32[2] = +fElapsedTime * m_Speed;
		m_Dir = EDirection::UP;
	}
	else if (move == 0x0c) {	// up right 1100
		m_Velocity.m128_f32[0] = +fElapsedTime * m_Speed * 0.70710678f;
		m_Velocity.m128_f32[2] = +fElapsedTime * m_Speed * 0.70710678f;
		m_Dir = EDirection::UPRIGHT;
	}
	else if (move == 0x04) {	// right 0100
		m_Velocity.m128_f32[0] = +fElapsedTime * m_Speed;
		m_Dir = EDirection::RIGHT;
	}
	else if (move == 0x06) {	// down right 0110
		m_Velocity.m128_f32[0] = +fElapsedTime * m_Speed * 0.70710678f;
		m_Velocity.m128_f32[2] = -fElapsedTime * m_Speed * 0.70710678f;
		m_Dir = EDirection::DOWNRIGHT;
	}
	else if (move == 0x02) {	// down 0010
		m_Velocity.m128_f32[2] = -fElapsedTime * m_Speed;
		m_Dir = EDirection::DOWN;
	}
	else if (move == 0x03) {	// down left 0011
		m_Velocity.m128_f32[0] = -fElapsedTime * m_Speed * 0.70710678f;
		m_Velocity.m128_f32[2] = -fElapsedTime * m_Speed * 0.70710678f;
		m_Dir = EDirection::DOWNLEFT;
	}
	else if (move == 0x01) {	// left 0001
		m_Velocity.m128_f32[0] = -fElapsedTime * m_Speed;
		m_Dir = EDirection::LEFT;
	}
	else if (move == 0x09) {	// up left 1001
		m_Velocity.m128_f32[0] = -fElapsedTime * m_Speed * 0.70710678f;
		m_Velocity.m128_f32[2] = +fElapsedTime * m_Speed * 0.70710678f;
		m_Dir = EDirection::UPLEFT;
	}

	// アニメーションの設定
	if (m_Velocity.m128_f32[0] || m_Velocity.m128_f32[2])
		this->ChangeAnimState((UINT)EAnimPlayer::RUN);
	else
		this->ChangeAnimState((UINT)EAnimPlayer::IDLE);

	// 位置の反映
	auto rotMat = XMMatrixRotationY(-m_CameraRotation.x - XM_PIDIV2);
	m_Velocity = XMVector3TransformCoord(m_Velocity, rotMat);
	m_Location += m_Velocity;
	// 回転の反映
	if (move) {
		m_Rotation.m128_f32[1] = static_cast<float>(m_Dir) * XM_PIDIV4 - m_CameraRotation.x - XM_PIDIV2;
	}

	// 落下
	if (m_isAir)
		m_Velocity.m128_f32[1] -= fElapsedTime * Gravity;

	// 値のリセット
	m_Velocity.m128_f32[0] = m_Velocity.m128_f32[2] = 0.0f;
	m_isAir = true;

	// カメラの更新
	UpdateCamera();

	AnimatedActor::FrameMove(time, fElapsedTime);
}

void Player::FrameRender(const double& time, const float& fElapsedTime)
{
	AnimatedActor::FrameRender(time, fElapsedTime);
}

void Player::OnCollide(SceneActor* other)
{
	if (wcscmp(other->GetName(), L"Ground") == 0) {
		m_Velocity.m128_f32[1] = 0.0f;
		m_isAir = false;
	}

	else if (wcscmp(other->GetName(), L"Enemy") == 0) {
		if (m_isDead == false)
		{
			m_isDead = true;
			this->SetStatus(RENDERONLY);
			other->SetStatus(RENDERONLY);
			XAudio2Manager::GetInstance()->StopBGM(0.0f);
			XAudio2Manager::GetInstance()->Play(L"Dead");
		}
	}
}

bool Player::IsDead() const
{
	return m_isDead;
}

void Player::InitCamera(const float x, const float y, const float d)
{
	// カメラ位置を設定する
	m_CameraDistance = d;
	m_CameraRotation.x	 = x;
	m_CameraRotation.y	 = y;

	float camX = static_cast<float>(d * cos(y) * cos(x));
	float camY = static_cast<float>(d * sin(y));
	float camZ = static_cast<float>(d * cos(y) * sin(x));
	m_CameraLocation = { camX, camY, camZ, 0.0f };
}

XMMATRIX Player::GetCameraView() const
{
	XMVECTOR upVec = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMVECTOR atVec = m_Location + XMVECTOR{ 0.0f, 5.0f, 0.0f, 0.0f };
	XMMATRIX view = XMMatrixLookAtLH(m_CameraLocation + m_Location, atVec, upVec);

	return view;
}

XMMATRIX Player::GetCameraLocation() const
{
	XMMATRIX location = XMMatrixTranslationFromVector(m_Location + m_CameraLocation);

	return location;
}

void Player::UpdateCamera()
{
	auto input = InputManager::GetInstance();
	if (input->GetKeyPressed("Mouse"))
	{
		GetCursorPos(&m_MouseStart);		// マウスのスクリーン座標取得
	}

	// マウスドラッグによるカメラ操作
	if (input->GetKeyState("Mouse"))
	{
		GetCursorPos(&m_MouseMove);

		//カメラ横方向角度変更
		m_CameraRotation.x += (m_MouseStart.x - m_MouseMove.x) * 0.003f;

		// カメラの縦方向角度に制限を設ける
		// 上制限
		if (m_CameraRotation.y - (m_MouseStart.y - m_MouseMove.y) * 0.003f >= XM_PI / 2.0f - 0.0001f)
		{
			m_CameraRotation.y = XM_PI / 2.0f - 0.0001f;	// カメラ縦方向角度変更
		}
		// 下制限
		else if (m_CameraRotation.y - (m_MouseStart.y - m_MouseMove.y) * 0.003f <= 0.1f)
		{
			m_CameraRotation.y = 0.1f;						// カメラ縦方向角度変更
		}
		else
		{
			m_CameraRotation.y -= (m_MouseStart.y - m_MouseMove.y) * 0.003f;//カメラ縦方向角度変更
		}

		// カメラ位置を設定する
		float camX = static_cast<float>(m_CameraDistance * cos(m_CameraRotation.y) * cos(m_CameraRotation.x));
		float camY = static_cast<float>(m_CameraDistance * sin(m_CameraRotation.y));
		float camZ = static_cast<float>(m_CameraDistance * cos(m_CameraRotation.y) * sin(m_CameraRotation.x));
		m_CameraLocation = { camX, camY, camZ, 0.0f };

		// マウス位置の固定
		SetCursorPos(m_MouseStart.x, m_MouseStart.y);
	}
}

HRESULT Player::LoadAnimation()
{
	HRESULT hr = S_OK;

	m_Animation = std::make_unique<Animation[]>(2);

	hr = m_Animation[0].LoadAnimation(L"Contents/Models/PlayerAnimation/Player_Idle.fbx", m_Model->GetBones());
	if (FAILED(hr))
		return hr;
	
	hr = m_Animation[1].LoadAnimation(L"Contents/Models/PlayerAnimation/Player_Running.fbx", m_Model->GetBones());
	if (FAILED(hr))
		return hr;

	return S_OK;
}