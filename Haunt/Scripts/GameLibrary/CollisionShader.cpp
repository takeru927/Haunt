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

	// ���_�V�F�[�_�̓ǂݍ���
	auto vertexShaderBlob = DX::ReadData(L"Shader\\CheckCollisionVS.cso");

	// ���_�V�F�[�_�̍쐬
	hr = device->CreateVertexShader(vertexShaderBlob.data(), vertexShaderBlob.size(),
		nullptr, m_VertexShader.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		vertexShaderBlob.clear();
		return hr;
	}

	// �C���v�b�g���C�A�E�g�̒�`
	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	// �C���v�b�g���C�A�E�g�̍쐬
	hr = device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexShaderBlob.data(),
		vertexShaderBlob.size(), m_InputLayout.ReleaseAndGetAddressOf());
	vertexShaderBlob.clear();
	if (FAILED(hr))
		return hr;

	// �s�N�Z���V�F�[�_�̓ǂݍ���
	auto pixelShaderBlob = DX::ReadData(L"Shader\\CheckCollisionPS.cso");

	// �s�N�Z���V�F�[�_�̍쐬
	hr = device->CreatePixelShader(pixelShaderBlob.data(), pixelShaderBlob.size(),
		nullptr, m_PixelShader.ReleaseAndGetAddressOf());
	pixelShaderBlob.clear();
	if (FAILED(hr))
		return hr;

	// ���X�^���C�U�̍쐬
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

	// �萔�o�b�t�@�̍쐬
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

	// �萔�o�b�t�@�̐ݒ�
	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	context->Map(m_CBMatrix.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	auto cbMatrix = (CBMatrix*)mappedResource.pData;
	cbMatrix->World		= XMMatrixTranspose(m_World);
	cbMatrix->View		= XMMatrixTranspose(m_View);
	cbMatrix->Projection = XMMatrixTranspose(m_Projection);
	context->Unmap(m_CBMatrix.Get(), 0);

	// �C���v�b�g���C�A�E�g�̐ݒ�
	context->IASetInputLayout(m_InputLayout.Get());

	// ���X�^���C�U�̐ݒ�
	context->RSSetState(m_RasterizerState.Get());

	// �V�F�[�_�̐ݒ�
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