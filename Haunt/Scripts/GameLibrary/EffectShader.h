#pragma once

using Microsoft::WRL::ComPtr;
using namespace DirectX;

__declspec(align(16))
class EffectShader
{
private:
	ComPtr<ID3D11VertexShader> m_VertexShader;
	ComPtr<ID3D11PixelShader>  m_PixelShader;
	ComPtr<ID3D11InputLayout>  m_InputLayout;
	ComPtr<ID3D11SamplerState> m_SamplerLinear;
	ComPtr<ID3D11BlendState>   m_AlphaBlend;

	ComPtr<ID3D11Buffer> m_CBMatrix;

	XMMATRIX m_World;
	XMMATRIX m_View;
	XMMATRIX m_Projection;

public:
	EffectShader();
	~EffectShader() = default;

	HRESULT CreateShader();
	void SetRenderShader();

	void SetWorld(XMMATRIX world);
	void SetView(XMMATRIX view);
	void SetProjection(XMMATRIX projection);

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(EffectShader));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};