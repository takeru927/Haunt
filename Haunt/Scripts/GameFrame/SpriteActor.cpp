#include "SpriteActor.h"
#include "GameLibrary/DirectDevice.h"

namespace
{
	struct SpriteVertex
	{
		XMFLOAT3 Pos;
		XMFLOAT2 TexUV;
	};
}

SpriteActor::SpriteActor(LPCWSTR name)
	: Actor(name), m_SpriteShader(nullptr), m_VertexBuffer(nullptr), m_TextureRV(nullptr)
	, m_Location({ 0.0f }), m_Rotation({ 0.0f }), m_Scale({ 1.0f, 1.0f, 1.0f, 1.0f })
	, m_SizeX(0.0f), m_SizeY(0.0f), m_ZBuffer(0.0f), m_Color({1.0f,1.0f,1.0f,1.0f}), m_FitWindowSize(false)
	, m_Pivot(ECtrlPoint::TOPLEFT), m_Anchor(ECtrlPoint::TOPLEFT)
{
}

void SpriteActor::Initialize(const float sizeX, const float sizeY, const float zBuffer, const bool fitSize,
							 const ECtrlPoint pivot, const ECtrlPoint anchor)
{
	m_SizeX			= sizeX;
	m_SizeY			= sizeY;
	m_ZBuffer		= zBuffer;
	m_FitWindowSize = fitSize;
	m_Pivot			= pivot;
	m_Anchor		= anchor;
}

HRESULT SpriteActor::CreateTextureResourse(LPCWSTR fileName)
{
	HRESULT hr = S_OK;

	hr = CreateWICTextureFromFileEx(
		D3D->GetDevice(),
		D3D->GetDeviceContext(),
		fileName,
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

	return S_OK;
}

HRESULT SpriteActor::CreateObject()
{
	HRESULT hr = S_OK;

	m_SpriteShader = std::make_unique<SpriteShader>();
	hr = m_SpriteShader->CreateShader();
	if (FAILED(hr))
		return hr;

	if (FAILED(this->CreateVertex()))
		return E_FAIL;

	if (FAILED(Actor::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void SpriteActor::FrameMove(const double& time, const float& fElapsedTime)
{
	Actor::FrameMove(time, fElapsedTime);
}

void SpriteActor::FrameRender(const double& time, const float& fElapsedTime)
{
	auto context = D3D->GetDeviceContext();

	XMMATRIX location = XMMatrixTranslationFromVector(this->CalcRealLocation());
	XMMATRIX rotation = XMMatrixRotationRollPitchYawFromVector(m_Rotation);
	XMMATRIX scale    = XMMatrixScalingFromVector(m_Scale);
	XMMATRIX screen   = scale * rotation * location;
	m_SpriteShader->SetScreen(screen);
	m_SpriteShader->SetMaterial(m_Color, m_TextureRV != nullptr);
	m_SpriteShader->SetRenderShader();

	UINT stride = sizeof(SpriteVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	if (m_TextureRV != nullptr)
		context->PSSetShaderResources(0, 1, m_TextureRV.GetAddressOf());
	else {
		ID3D11ShaderResourceView* srv[] = { nullptr };
		context->PSSetShaderResources(0, 1, srv);
	}

	context->Draw(4, 0);

	Actor::FrameRender(time, fElapsedTime);
}

void SpriteActor::OnResizedSwapChain()
{
	auto screen = D3D->GetScreenViewport();
	m_SpriteShader->SetProjection(screen.Width, screen.Height);

	// ウィンドウサイズに合わせてスケールを調整する
	if (m_FitWindowSize)
	{
		m_Scale.m128_f32[0] = screen.Width / m_SizeX;
		m_Scale.m128_f32[1] = screen.Height / m_SizeY;
	}

	Actor::OnResizedSwapChain();
}

void SpriteActor::SetLocation(XMVECTOR location)
{
	m_Location = location;
}

void SpriteActor::SetRotation(XMVECTOR rotation)
{
	m_Rotation = rotation;
}

void SpriteActor::SetScale(XMVECTOR scale)
{
	m_Scale = scale;
}

HRESULT SpriteActor::CreateVertex()
{
	HRESULT hr = S_OK;

	// 中心点の設定
	float pivotX = 0.0f, pivotY = 0.0f;
	switch (m_Pivot)
	{
	case ECtrlPoint::TOPLEFT:
		pivotX = 0.0f;
		pivotY = 0.0f;
		break;

	case ECtrlPoint::TOP:
		pivotX = m_SizeX * 0.5f;
		pivotY = 0.0f;
		break;

	case ECtrlPoint::TOPRIGHT:
		pivotX = m_SizeX;
		pivotY = 0.0f;
		break;

	case ECtrlPoint::RIGHT:
		pivotX = m_SizeX;
		pivotY = m_SizeY * 0.5f;
		break;

	case ECtrlPoint::BOTTOMRIGHT:
		pivotX = m_SizeX;
		pivotY = m_SizeY;
		break;

	case ECtrlPoint::BOTTOM:
		pivotX = m_SizeX * 0.5f;
		pivotY = m_SizeY;
		break;

	case ECtrlPoint::BOTTOMLEFT:
		pivotX = 0.0f;
		pivotY = m_SizeY;
		break;

	case ECtrlPoint::LEFT:
		pivotX = 0.0f;
		pivotY = m_SizeY * 0.5f;
		break;

	case ECtrlPoint::CENTER:
		pivotX = m_SizeX * 0.5f;
		pivotY = m_SizeY * 0.5f;
		break;
	}

	SpriteVertex vertices[] = {
		{{ -pivotX			, -pivotY		  , m_ZBuffer }, { 0.0f, 0.0f }},
		{{ m_SizeX - pivotX , -pivotY		  , m_ZBuffer }, { 1.0f, 0.0f }},
		{{ -pivotX			, m_SizeY - pivotY, m_ZBuffer }, { 0.0f, 1.0f }},
		{{ m_SizeX - pivotX , m_SizeY - pivotY, m_ZBuffer }, { 1.0f, 1.0f }},
	};

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(SpriteVertex) * 4;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertices;
	hr = D3D->GetDevice()->CreateBuffer(&bd, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

// 左上(原点)を基準とした位置を求める
XMVECTOR SpriteActor::CalcRealLocation()
{
	XMVECTOR realLocation = m_Location;

	auto screen = D3D->GetScreenViewport();
	switch (m_Anchor)
	{
	case ECtrlPoint::TOPLEFT:
		break;

	case ECtrlPoint::TOP:
		realLocation.m128_f32[0] = screen.Width * 0.5f + realLocation.m128_f32[0];
		break;

	case ECtrlPoint::TOPRIGHT:
		realLocation.m128_f32[0] = screen.Width + realLocation.m128_f32[0];
		break;

	case ECtrlPoint::RIGHT:
		realLocation.m128_f32[0] = screen.Width + realLocation.m128_f32[0];
		realLocation.m128_f32[1] = screen.Height * 0.5f + realLocation.m128_f32[1];
		break;

	case ECtrlPoint::BOTTOMRIGHT:
		realLocation.m128_f32[0] = screen.Width + realLocation.m128_f32[0];
		realLocation.m128_f32[1] = screen.Height + realLocation.m128_f32[1];
		break;

	case ECtrlPoint::BOTTOM:
		realLocation.m128_f32[0] = screen.Width * 0.5f + realLocation.m128_f32[0];
		realLocation.m128_f32[1] = screen.Height + realLocation.m128_f32[1];
		break;

	case ECtrlPoint::BOTTOMLEFT:
		realLocation.m128_f32[1] = screen.Height + realLocation.m128_f32[1];
		break;

	case ECtrlPoint::LEFT:
		realLocation.m128_f32[1] = screen.Height * 0.5f + realLocation.m128_f32[1];
		break;

	case ECtrlPoint::CENTER:
		realLocation.m128_f32[0] = screen.Width * 0.5f + realLocation.m128_f32[0];
		realLocation.m128_f32[1] = screen.Height * 0.5f + realLocation.m128_f32[1];
		break;
	}

	return realLocation;
}