#include "Shader.h"
#include "ReadData.h"
#include "DirectDevice.h"

namespace
{
	struct CBMatrix
	{
		XMMATRIX World;
		XMMATRIX View;
		XMMATRIX Projection;
	};

	struct CBMaterial
	{
		XMFLOAT4 Ambient;
		XMFLOAT4 Diffuse;
		float    UseTexture;
		XMFLOAT3 dummy;
	};

	struct CBLighting
	{
		XMFLOAT4 LightDir;
		XMFLOAT4 LightAmbient;
		XMFLOAT4 LightDiffuse;
	};
}

Shader::Shader()
	: m_VertexShader(nullptr), m_PixelShader(nullptr), m_InputLayout(nullptr)
	, m_SamplerLinear(nullptr), m_RasterizerState(nullptr), m_NormalBlend(nullptr)
	, m_CBMatrix(nullptr), m_CBMaterial(nullptr), m_CBLighting(nullptr)
	, m_World(XMMatrixIdentity()), m_View(XMMatrixIdentity()), m_Projection(XMMatrixIdentity())
{
}

HRESULT Shader::CreateShader()
{
	HRESULT hr = S_OK;

	auto device = D3D->GetDevice();

	auto vertexShaderGraph = DX::ReadData(L"Shader\\StandardVS.cso");
	hr = device->CreateVertexShader(vertexShaderGraph.data(), vertexShaderGraph.size(),
		nullptr, m_VertexShader.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		vertexShaderGraph.clear();
		return hr;
	}

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION",  0, DXGI_FORMAT_R32G32B32_FLOAT,	 0,							   0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"NORMAL",	  0, DXGI_FORMAT_R32G32B32_FLOAT,	 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",  0, DXGI_FORMAT_R32G32_FLOAT,		 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexShaderGraph.data(),
		vertexShaderGraph.size(), m_InputLayout.ReleaseAndGetAddressOf());
	vertexShaderGraph.clear();
	if (FAILED(hr))
		return hr;

	auto pixelShaderGraph = DX::ReadData(L"Shader\\StandardPS.cso");
	hr = device->CreatePixelShader(pixelShaderGraph.data(), pixelShaderGraph.size(),
		nullptr, m_PixelShader.ReleaseAndGetAddressOf());
	pixelShaderGraph.clear();
	if (FAILED(hr))
		return hr;

	D3D11_SAMPLER_DESC samplerDesc = {};
	samplerDesc.Filter			= D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW		= D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias		= 0.0f;
	samplerDesc.MaxAnisotropy	= 1;
	samplerDesc.ComparisonFunc	= D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD			= 0;
	samplerDesc.MaxLOD			= D3D11_FLOAT32_MAX;
	hr = device->CreateSamplerState(&samplerDesc, m_SamplerLinear.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_RASTERIZER_DESC rasDesc = {};
	rasDesc.FillMode = D3D11_FILL_SOLID;
	rasDesc.CullMode = D3D11_CULL_FRONT;
	rasDesc.FrontCounterClockwise = TRUE;
	hr = device->CreateRasterizerState(&rasDesc, m_RasterizerState.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_BLEND_DESC blendState = {};
	blendState.AlphaToCoverageEnable = FALSE;
	blendState.IndependentBlendEnable = FALSE;
	blendState.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	blendState.RenderTarget[0].BlendEnable = FALSE;
	hr = device->CreateBlendState(&blendState, m_NormalBlend.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_BUFFER_DESC constantBuffer = {};
	constantBuffer.Usage		  = D3D11_USAGE_DYNAMIC;
	constantBuffer.BindFlags	  = D3D11_BIND_CONSTANT_BUFFER;
	constantBuffer.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantBuffer.MiscFlags	  = 0;
	constantBuffer.ByteWidth = sizeof(CBMatrix);
	hr = device->CreateBuffer(&constantBuffer, nullptr, m_CBMatrix.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;
	constantBuffer.ByteWidth = sizeof(CBMaterial);
	hr = device->CreateBuffer(&constantBuffer, nullptr, m_CBMaterial.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;
	constantBuffer.ByteWidth = sizeof(CBLighting);
	hr = device->CreateBuffer(&constantBuffer, nullptr, m_CBLighting.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void Shader::SetRenderShader()
{
	auto context = D3D->GetDeviceContext();

	// 定数バッファの設定
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	context->Map(m_CBMatrix.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto cbMatrix = (CBMatrix*)mappedResource.pData;
	cbMatrix->World		 = XMMatrixTranspose(m_World);
	cbMatrix->View		 = XMMatrixTranspose(m_View);
	cbMatrix->Projection = XMMatrixTranspose(m_Projection);
	context->Unmap(m_CBMatrix.Get(), 0);

	context->IASetInputLayout(m_InputLayout.Get());
	context->RSSetState(m_RasterizerState.Get());
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers(0, 1, m_CBMatrix.GetAddressOf());
	context->VSSetConstantBuffers(1, 1, m_CBMaterial.GetAddressOf());
	context->VSSetConstantBuffers(2, 1, m_CBLighting.GetAddressOf());

	context->PSSetShader(m_PixelShader.Get(), nullptr, 0);
	context->PSSetConstantBuffers(1, 1, m_CBMaterial.GetAddressOf());
	context->PSSetSamplers(0, 1, m_SamplerLinear.GetAddressOf());

	FLOAT BlendFactor[4] = { 0.0f };
	context->OMSetBlendState(m_NormalBlend.Get(), BlendFactor, 0xFFFFFFFF);
}

void Shader::SetWorld(XMMATRIX world)
{
	m_World = world;
}

void Shader::SetView(XMMATRIX view)
{
	m_View = view;
}

void Shader::SetProjection(XMMATRIX projection)
{
	m_Projection = projection;
}

void Shader::SetDirectionalLight(XMFLOAT4 lightDir, XMFLOAT4 lightAmbient, XMFLOAT4 lightDiffuse)
{
	auto context = D3D->GetDeviceContext();

	// 定数バッファの設定
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	context->Map(m_CBLighting.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto cbLighting = (CBLighting*)mappedResource.pData;
	cbLighting->LightDir     = lightDir;
	cbLighting->LightAmbient = lightAmbient;
	cbLighting->LightDiffuse = lightDiffuse;
	context->Unmap(m_CBLighting.Get(), 0);
}

void Shader::SetMaterial(XMFLOAT4 ambient, XMFLOAT4 diffuse, bool useTex)
{
	auto context = D3D->GetDeviceContext();

	// 定数バッファの設定
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	context->Map(m_CBMaterial.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto cbMaterial = (CBMaterial*)mappedResource.pData;
	cbMaterial->Ambient    = ambient;
	cbMaterial->Diffuse	   = diffuse;
	cbMaterial->UseTexture = useTex ? 1.0f : 0.0f;
	context->Unmap(m_CBMaterial.Get(), 0);
}