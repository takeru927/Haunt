#pragma once
#include "GameFrame/Actor.h"
#include "GameLibrary/CollisionShader.h"

class Collision;

class CheckCollision : public Actor
{
private:
	std::unique_ptr<CollisionShader> m_CollisionShader;
	Collision*						 m_Collision;

	ComPtr<ID3D11Buffer>	 m_VertexBuffer;
	ComPtr<ID3D11Buffer>	 m_IndexBuffer;
	D3D11_PRIMITIVE_TOPOLOGY m_PrimitiveTopology;

	UINT m_IndexNum;

public:
	CheckCollision(Collision* const collision);
	~CheckCollision() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;

	HRESULT CreateCapsuleVertex(float radius, float height);
	HRESULT CreateBoxVertex(XMVECTOR extent);
};