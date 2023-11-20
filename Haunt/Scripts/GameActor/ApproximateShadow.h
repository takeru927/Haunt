#pragma once
#include "GameFrame/SceneActor.h"
#include "GameLibrary/ShadowShader.h"

class ApproximateShadow : public SceneActor
{
private:
	std::unique_ptr<ShadowShader> m_Shader;

	ComPtr<ID3D11ShaderResourceView> m_TextureRV;
	ComPtr<ID3D11Buffer>			 m_VertexBuffer;
	ComPtr<ID3D11DepthStencilState>  m_DepthNone;

	SceneActor* m_Actor;
	float m_Size;

public:
	ApproximateShadow(SceneActor* actor, float scale = 1.0f);
	~ApproximateShadow() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;
};