#pragma once
#include "GameFrame/Actor.h"
#include "GameLibrary/SkyBoxShader.h"

class Scene;

constexpr int SKYBOXSIDES = 6;

class SkyBox : public Actor
{
private:
	WCHAR							 m_FileName[SKYBOXSIDES][256];
	std::unique_ptr<SkyBoxShader>	 m_Shader;

	ComPtr<ID3D11Buffer>			 m_VertexBuffer;
	ComPtr<ID3D11Buffer>			 m_IndexBuffer;
	ComPtr<ID3D11DepthStencilState>  m_DepthNone;
	ComPtr<ID3D11ShaderResourceView> m_TextureRV;

	UINT	m_IndexNum;

public:
	SkyBox(const LPCWSTR fileName);
	~SkyBox() = default;

	HRESULT CreateObject() override;
	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;
};