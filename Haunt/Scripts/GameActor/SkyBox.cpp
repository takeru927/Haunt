#include "SkyBox.h"
#include "GameLibrary/WICTextureLoader11.h"
#include "GameLibrary/DirectDevice.h"

#include "GameMain.h"

SkyBox::SkyBox(const LPCWSTR folderName)
	: Actor(L"SkyBox")
	, m_VertexBuffer(nullptr), m_Shader(nullptr)
	, m_IndexBuffer(nullptr), m_DepthNone(nullptr), m_TextureRV(nullptr), m_IndexNum(0)
{
	for (int i = 0; i < SKYBOXSIDES; i++)
		wcscpy_s(m_FileName[i], folderName);

	wcscat_s(m_FileName[0], L"/posX.png");
	wcscat_s(m_FileName[1], L"/negX.png");
	wcscat_s(m_FileName[2], L"/posY.png");
	wcscat_s(m_FileName[3], L"/negY.png");
	wcscat_s(m_FileName[4], L"/posZ.png");
	wcscat_s(m_FileName[5], L"/negZ.png");
}

HRESULT SkyBox::CreateObject()
{
	HRESULT hr = S_OK;

	m_Shader = std::make_unique<SkyBoxShader>();
	hr = m_Shader->CreateShader();
	if (FAILED(hr))
		return hr;

	const float scale = 1.0f;
	XMFLOAT3 vertex[] = {
		// Front
		{-scale,  scale,  scale},
		{ scale,  scale,  scale},
		{-scale, -scale,  scale},
		{ scale, -scale,  scale},
		// Back
		{-scale,  scale, -scale},
		{ scale,  scale, -scale},
		{-scale, -scale, -scale},
		{ scale, -scale, -scale}
	};

	WORD indices[] =
	{
		//Front
		0, 1, 3,
		3, 2, 0,
		//Back
		5, 4, 6,
		6, 7, 5,
		//Left
		4, 0, 2,
		2, 6, 4,
		//Right
		1, 5, 7,
		7, 3, 1,
		//Top
		4, 5, 1,
		1, 0, 4,
		//Bottom
		2, 3, 7,
		7, 6, 2
	};

	auto device = D3D->GetDevice();

	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.Usage = D3D11_USAGE_DEFAULT;
	vbDesc.ByteWidth = sizeof(XMFLOAT3) * ARRAYSIZE(vertex);
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertex;
	hr = device->CreateBuffer(&vbDesc, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	m_IndexNum = ARRAYSIZE(indices);
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(WORD) * m_IndexNum;
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	InitData.pSysMem = indices;
	hr = device->CreateBuffer(&ibDesc, &InitData, m_IndexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
	hr = device->CreateDepthStencilState(&dsDesc, m_DepthNone.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	ID3D11Texture2D* texture[SKYBOXSIDES] = {};
	for (int i = 0; i < SKYBOXSIDES; i++)
	{
		hr = CreateWICTextureFromFileEx(
			device,
			D3D->GetDeviceContext(),
			m_FileName[i],
			0,
			D3D11_USAGE_STAGING,
			0,
			D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE, 
			0,
			WIC_LOADER_DEFAULT,
			(ID3D11Resource**)&texture[i],
			nullptr
		);
		if (FAILED(hr))
			return hr;
	}
	
	D3D11_TEXTURE2D_DESC texElementDesc = {};
	texture[0]->GetDesc(&texElementDesc);

	D3D11_TEXTURE2D_DESC texArrayDesc = {};
	texArrayDesc.Height				= texElementDesc.Height;
	texArrayDesc.Width				= texElementDesc.Width;
	texArrayDesc.ArraySize			= SKYBOXSIDES;
	texArrayDesc.BindFlags			= D3D11_BIND_SHADER_RESOURCE;
	texArrayDesc.CPUAccessFlags		= 0;
	texArrayDesc.Format				= texElementDesc.Format;
	texArrayDesc.MipLevels			= texElementDesc.MipLevels;
	texArrayDesc.MiscFlags			= D3D11_RESOURCE_MISC_TEXTURECUBE;
	texArrayDesc.SampleDesc.Count	= 1;
	texArrayDesc.SampleDesc.Quality = 0;
	texArrayDesc.Usage				= D3D11_USAGE_DEFAULT;
	ID3D11Texture2D* combinedTextures = nullptr;
	hr = device->CreateTexture2D(&texArrayDesc, nullptr, &combinedTextures);
	if (FAILED(hr))
		return hr;

	auto context = D3D->GetDeviceContext();
	D3D11_SUBRESOURCE_DATA data[SKYBOXSIDES] = {};
	for (int i = 0; i < SKYBOXSIDES; i++)
	{
		for (UINT mipLevel = 0; mipLevel < texElementDesc.MipLevels; mipLevel++)
		{
			D3D11_MAPPED_SUBRESOURCE mappedTex = {};
			hr = context->Map(texture[i], mipLevel, D3D11_MAP_READ, 0, &mappedTex);
			if (FAILED(hr))
				return hr;

			UINT srIndex = D3D11CalcSubresource(mipLevel, i, texElementDesc.MipLevels);
			context->UpdateSubresource(combinedTextures, srIndex, 0,
				mappedTex.pData, mappedTex.RowPitch, mappedTex.DepthPitch);

			context->Unmap(texture[i], mipLevel);
		}
	}

	for (int i = 0; i < SKYBOXSIDES; i++)
		texture[i]->Release();

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = texElementDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	srvDesc.TextureCube.MipLevels = texElementDesc.MipLevels;
	srvDesc.TextureCube.MostDetailedMip = 0;
	hr = device->CreateShaderResourceView(combinedTextures, &srvDesc, m_TextureRV.ReleaseAndGetAddressOf());
	combinedTextures->Release();
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void SkyBox::FrameMove(const double& time, const float& fElapsedTime)
{
	Actor::FrameMove(time, fElapsedTime);
}

void SkyBox::FrameRender(const double& time, const float& fElapsedTime)
{
	XMMATRIX view  = GAME->GetCameraView();
	XMMATRIX world = GAME->GetCameraLocation();
	m_Shader->SetView(view);
	m_Shader->SetWorld(world);

	auto context = D3D->GetDeviceContext();
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->PSSetShaderResources(0, 1, m_TextureRV.GetAddressOf());
	context->OMSetDepthStencilState(m_DepthNone.Get(), 0);

	m_Shader->SetRenderShader();

	context->DrawIndexed(m_IndexNum, 0, 0);

	context->OMSetDepthStencilState(nullptr, 0);

	Actor::FrameRender(time, fElapsedTime);
}

void SkyBox::OnResizedSwapChain()
{
	// ŽË‰es—ñ‚ÌÝ’è
	auto screen = D3D->GetScreenViewport();
	auto aspect = screen.Width / screen.Height;
	auto projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 0.01f, 1000.0f);
	m_Shader->SetProjection(projection);

	Actor::OnResizedSwapChain();
}