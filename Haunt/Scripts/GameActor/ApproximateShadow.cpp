#include "ApproximateShadow.h"
#include "GameLibrary/DirectDevice.h"

#include "GameMain.h"

namespace
{
	struct ShadowVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 TexUV;
	};

	constexpr float GroundHeight = 0.5f;
	constexpr float ShadowHeight = GroundHeight + 0.05f;
}

ApproximateShadow::ApproximateShadow(SceneActor* actor, float scale)
	: SceneActor(L"Shadow"), m_Shader(nullptr), m_TextureRV(nullptr), m_VertexBuffer(nullptr), m_DepthNone(nullptr)
	, m_Actor(actor), m_Size(scale)
{
}

HRESULT ApproximateShadow::CreateObject()
{
	HRESULT hr = S_OK;

	m_Shader = std::make_unique<ShadowShader>();
	hr = m_Shader->CreateShader();
	if (FAILED(hr))
		return hr;

	// テクスチャの読み込み
	hr = CreateWICTextureFromFileEx(
		D3D->GetDevice(),
		D3D->GetDeviceContext(),
		L"Contents/Textures/shadow.png",
		0,
		D3D11_USAGE_DEFAULT,
		D3D11_BIND_SHADER_RESOURCE,
		0, 0,
		WIC_LOADER_FORCE_SRGB,
		nullptr,
		m_TextureRV.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
		return hr;

	ShadowVertex vertices[] = {
		{{ -m_Size, 0.0f,  m_Size }, { 0.0f, 0.0f }},
		{{  m_Size, 0.0f,  m_Size }, { 1.0f, 0.0f }},
		{{ -m_Size, 0.0f, -m_Size }, { 0.0f, 1.0f }},
		{{  m_Size, 0.0f, -m_Size }, { 1.0f, 1.0f }},
	};

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(ShadowVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = D3D->GetDevice()->CreateBuffer(&bd, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = D3D->GetDevice()->CreateDepthStencilState(&dsDesc, m_DepthNone.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void ApproximateShadow::FrameMove(const double& time, const float& fElapsedTime)
{
	Actor::FrameMove(time, fElapsedTime);
}

void ApproximateShadow::FrameRender(const double& time, const float& fElapsedTime)
{
	auto context = D3D->GetDeviceContext();

	XMMATRIX view = GAME->GetCameraView();
	m_Shader->SetView(view);

	this->m_Location = m_Actor->GetLocation();
	this->m_Location.m128_f32[1] = ShadowHeight;
	m_World = XMMatrixTranslationFromVector(m_Location);
	m_Shader->SetWorld(m_World);

	m_Shader->SetRenderShader();

	UINT stride = sizeof(ShadowVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	context->PSSetShaderResources(0, 1, m_TextureRV.GetAddressOf());
	context->OMSetDepthStencilState(m_DepthNone.Get(), 0);

	context->Draw(4, 0);

	// 深度バッファの設定を戻す
	context->OMSetDepthStencilState(nullptr, 0);

	Actor::FrameRender(time, fElapsedTime);
}

void ApproximateShadow::OnResizedSwapChain()
{
	// 射影行列の設定
	auto screen = D3D->GetScreenViewport();
	auto aspect = screen.Width / screen.Height;
	auto projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 0.01f, 1000.0f);
	m_Shader->SetProjection(projection);

	Actor::OnResizedSwapChain();
}