#pragma once
#include "Actor.h"
#include "GameLibrary/SpriteShader.h"

using namespace DirectX;

enum class ECtrlPoint { TOPLEFT, TOP, TOPRIGHT, RIGHT, BOTTOMRIGHT, BOTTOM, BOTTOMLEFT, LEFT, CENTER };

__declspec(align(16))
class SpriteActor : public Actor
{
protected:
	std::unique_ptr<SpriteShader>	 m_SpriteShader;
	ComPtr<ID3D11Buffer>			 m_VertexBuffer;
	ComPtr<ID3D11ShaderResourceView> m_TextureRV;

	XMVECTOR m_Location;
	XMVECTOR m_Rotation;
	XMVECTOR m_Scale;
	float	 m_SizeX;
	float	 m_SizeY;
	float	 m_ZBuffer;
	XMFLOAT4 m_Color;

	bool	   m_FitWindowSize;
	ECtrlPoint m_Pivot;
	ECtrlPoint m_Anchor;

public:
	SpriteActor(LPCWSTR name);
	virtual ~SpriteActor() = default;

	void Initialize(const float sizeX, const float sizeY, const float zBuffer, const bool fitSize = false,
		const ECtrlPoint pivot = ECtrlPoint::TOPLEFT, const ECtrlPoint anchor = ECtrlPoint::TOPLEFT);
	HRESULT CreateTextureResourse(LPCWSTR fileName);
	virtual HRESULT CreateObject();
	virtual void FrameMove(const double& time, const float& fElapsedTime);
	virtual void FrameRender(const double& time, const float& fElapsedTime);

	virtual void OnResizedSwapChain();

	void SetLocation(XMVECTOR location);
	void SetRotation(XMVECTOR rotation);
	void SetScale(XMVECTOR scale);

private:
	HRESULT CreateVertex();
	XMVECTOR CalcRealLocation();

public:
	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(SpriteActor));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};