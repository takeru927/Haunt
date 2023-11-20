#include "EffectShader.h"
#include "DirectDevice.h"
#include "ReadData.h"

namespace
{
	struct CBMatrix
	{
		XMMATRIX matWorld;
		XMMATRIX matView;
		XMMATRIX matProjection;
	};
}

EffectShader::EffectShader()
	: m_VertexShader(nullptr), m_PixelShader(nullptr), m_InputLayout(nullptr)
	, m_SamplerLinear(nullptr), m_AlphaBlend(nullptr), m_CBMatrix(nullptr)
	, m_World(XMMatrixIdentity()), m_View(XMMatrixIdentity()), m_Projection(XMMatrixIdentity())
{
}

HRESULT EffectShader::CreateShader()
{
	HRESULT hr = S_OK;

	auto device = D3D->GetDevice();

	// 頂点シェーダの読み込み
	auto vertexShaderBlob = DX::ReadData(L"Shader\\EffectVS.cso");

	// 頂点シェーダの作成
	hr = device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
		nullptr, m_VertexShader.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		vertexShaderBlob.clear();
		return hr;
	}

	// インプットレイアウトの定義
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT   , 0,							  0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR"   , 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT      , 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// インプットレイアウトの作成
	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexShaderBlob.data(),
		vertexShaderBlob.size(), m_InputLayout.ReleaseAndGetAddressOf());
	vertexShaderBlob.clear();
	if (FAILED(hr))
		return hr;

	// ピクセルシェーダの読み込み
	auto pixelShaderBlob = DX::ReadData(L"Shader\\EffectPS.cso");

	// ピクセルシェーダの作成
	hr = device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
		nullptr, m_PixelShader.ReleaseAndGetAddressOf());
	pixelShaderBlob.clear();
	if (FAILED(hr))
		return hr;

	// サンプラーの作成
	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter		   = D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
	samplerDesc.AddressU	   = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV	   = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW	   = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MaxAnisotropy  = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MaxLOD		   = D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&samplerDesc, m_SamplerLinear.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	// アルファブレンドの作成
	D3D11_BLEND_DESC blendState = {};
	blendState.AlphaToCoverageEnable				 = FALSE;
	blendState.IndependentBlendEnable				 = FALSE;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendState.RenderTarget[0].BlendEnable			 = TRUE;
	blendState.RenderTarget[0].SrcBlend				 = D3D11_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend			 = D3D11_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp				 = D3D11_BLEND_OP_ADD;
	blendState.RenderTarget[0].SrcBlendAlpha		 = D3D11_BLEND_ZERO;
	blendState.RenderTarget[0].DestBlendAlpha		 = D3D11_BLEND_ZERO;
	blendState.RenderTarget[0].BlendOpAlpha			 = D3D11_BLEND_OP_ADD;
	hr = device->CreateBlendState(&blendState, m_AlphaBlend.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	// 定数バッファの作成
	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.Usage = D3D11_USAGE_DYNAMIC;
	bufDesc.ByteWidth = sizeof(CBMatrix);
	bufDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	hr = device->CreateBuffer(&bufDesc, nullptr, m_CBMatrix.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void EffectShader::SetRenderShader()
{
	auto context = D3D->GetDeviceContext();

	// 定数バッファの設定
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	context->Map(m_CBMatrix.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto cbMatrix = (CBMatrix*)mappedResource.pData;
	cbMatrix->matWorld = XMMatrixTranspose(m_World);
	cbMatrix->matView = XMMatrixTranspose(m_View);
	cbMatrix->matProjection = XMMatrixTranspose(m_Projection);
	context->Unmap(m_CBMatrix.Get(), 0);

	// シェーダの設定
	context->IASetInputLayout(m_InputLayout.Get());
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers(0, 1, m_CBMatrix.GetAddressOf());
	context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
	context->PSSetSamplers(0, 1, m_SamplerLinear.GetAddressOf());

	FLOAT BlendFactor[4] = { 0.0f };
	context->OMSetBlendState(m_AlphaBlend.Get(), BlendFactor, 0xFFFFFFFF);
}

void EffectShader::SetWorld(XMMATRIX world)
{
	m_World = world;
}

void EffectShader::SetView(XMMATRIX view)
{
	m_View = view;
}

void EffectShader::SetProjection(XMMATRIX projection)
{
	m_Projection = projection;
}