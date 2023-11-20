#include "CollisionShader.h"
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
}

CollisionShader::CollisionShader()
	: m_VertexShader(nullptr), m_PixelShader(nullptr), m_InputLayout(nullptr)
	, m_RasterizerState(nullptr), m_NormalBlend(nullptr), m_CBMatrix(nullptr)
	, m_World(XMMatrixIdentity()), m_View(XMMatrixIdentity()), m_Projection(XMMatrixIdentity())
{
}

HRESULT CollisionShader::CreateShader()
{
	HRESULT hr = S_OK;

	auto device = D3D->GetDevice();

	// 頂点シェーダの読み込み
	auto vertexShaderBlob = DX::ReadData(L"Shader\\CheckCollisionVS.cso");

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
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// インプットレイアウトの作成
	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexShaderBlob.data(),
		vertexShaderBlob.size(), m_InputLayout.ReleaseAndGetAddressOf());
	vertexShaderBlob.clear();
	if (FAILED(hr))
		return hr;

	// ピクセルシェーダの読み込み
	auto pixelShaderBlob = DX::ReadData(L"Shader\\CheckCollisionPS.cso");

	// ピクセルシェーダの作成
	hr = device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
		nullptr, m_PixelShader.ReleaseAndGetAddressOf());
	pixelShaderBlob.clear();
	if (FAILED(hr))
		return hr;

	// ラスタライザの作成
	D3D11_RASTERIZER_DESC rasDesc = {};
	rasDesc.FillMode = D3D11_FILL_WIREFRAME;
	rasDesc.CullMode = D3D11_CULL_NONE;
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

	// 定数バッファの作成
	D3D11_BUFFER_DESC bufDesc = {};
	bufDesc.Usage			= D3D11_USAGE_DYNAMIC;
	bufDesc.BindFlags		= D3D11_BIND_CONSTANT_BUFFER;
	bufDesc.CPUAccessFlags  = D3D11_CPU_ACCESS_WRITE;
	bufDesc.MiscFlags		= 0;
	bufDesc.ByteWidth		= sizeof(CBMatrix);
	hr = device->CreateBuffer(&bufDesc, nullptr, m_CBMatrix.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void CollisionShader::SetRenderShader()
{
	auto context = D3D->GetDeviceContext();

	// 定数バッファの設定
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	context->Map(m_CBMatrix.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto cbMatrix = (CBMatrix*)mappedResource.pData;
	cbMatrix->World		= XMMatrixTranspose(m_World);
	cbMatrix->View		= XMMatrixTranspose(m_View);
	cbMatrix->Projection = XMMatrixTranspose(m_Projection);
	context->Unmap(m_CBMatrix.Get(), 0);

	// インプットレイアウトの設定
	context->IASetInputLayout(m_InputLayout.Get());

	// ラスタライザの設定
	context->RSSetState(m_RasterizerState.Get());

	// シェーダの設定
	context->VSSetShader(m_VertexShader.Get(), nullptr, 0);
	context->VSSetConstantBuffers(0, 1, m_CBMatrix.GetAddressOf());

	context->PSSetShader(m_PixelShader.Get(), nullptr, 0);

	FLOAT BlendFactor[4] = { 0.0f };
	context->OMSetBlendState(m_NormalBlend.Get(), BlendFactor, 0xFFFFFFFF);
}

void CollisionShader::SetWorld(XMMATRIX world)
{
	m_World = world;
}

void CollisionShader::SetView(XMMATRIX view)
{
	m_View = view;
}

void CollisionShader::SetProjection(XMMATRIX projection)
{
	m_Projection = projection;
}