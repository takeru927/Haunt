#pragma once

using Microsoft::WRL::ComPtr;

class SpriteShader
{
private:
	ComPtr<ID3D11VertexShader>	  m_VertexShader;
	ComPtr<ID3D11PixelShader>	  m_PixelShader;
	ComPtr<ID3D11InputLayout>	  m_InputLayout;
	ComPtr<ID3D11SamplerState>	  m_SamplerLinear;
	ComPtr<ID3D11RasterizerState> m_RasterizerState;
	ComPtr<ID3D11BlendState>	  m_AlphaBlend;

	ComPtr<ID3D11Buffer>		  m_CBMatrix;
	ComPtr<ID3D11Buffer>		  m_CBMaterial;

	DirectX::XMMATRIX			  m_Screen;
	DirectX::XMMATRIX			  m_Projection;

public:
	SpriteShader();
	~SpriteShader() = default;

	HRESULT CreateShader();
	void SetRenderShader();

	void SetScreen(DirectX::XMMATRIX screen);
	void SetProjection(DirectX::XMMATRIX projection);
	void SetProjection(float width, float height);
	void SetMaterial(DirectX::XMFLOAT4 color, bool useTex);
};