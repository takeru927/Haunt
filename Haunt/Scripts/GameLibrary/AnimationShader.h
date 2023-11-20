#pragma once

using Microsoft::WRL::ComPtr;
using namespace DirectX;

__declspec(align(16))
class AnimationShader
{
private:
	ComPtr<ID3D11VertexShader>	  m_VertexShader;
	ComPtr<ID3D11PixelShader>	  m_PixelShader;
	ComPtr<ID3D11InputLayout>	  m_InputLayout;
	ComPtr<ID3D11SamplerState>	  m_SamplerLinear;
	ComPtr<ID3D11RasterizerState> m_RasterizerState;
	ComPtr<ID3D11BlendState>	  m_NormalBlend;

	ComPtr<ID3D11Buffer>		m_CBMatrix;
	ComPtr<ID3D11Buffer>		m_CBMaterial;
	ComPtr<ID3D11Buffer>		m_CBLighting;
	ComPtr<ID3D11Buffer>		m_CBBones;

	XMMATRIX m_World;
	XMMATRIX m_View;
	XMMATRIX m_Projection;

public:
	AnimationShader();
	~AnimationShader() = default;

	HRESULT CreateShader();
	void SetRenderShader();

	void SetWorld(XMMATRIX world);
	void SetView(XMMATRIX view);
	void SetProjection(XMMATRIX projection);
	void SetDirectionalLight(XMFLOAT4 lightDir, XMFLOAT4 lightAmbient, XMFLOAT4 lightDiffuse);
	void SetMaterial(XMFLOAT4 ambient, XMFLOAT4 diffuse, bool useTex);
	void SetBones(const XMMATRIX bones[]);

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(AnimationShader));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};