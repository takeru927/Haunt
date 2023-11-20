#include "DirectDevice.h"
#include "ReadData.h"

#pragma comment(lib,"dxgi.lib")

#define SAFE_RELEASE(p) { if(p) {(p).Reset(); (p) = nullptr; } }
DirectDevice* DirectDevice::instance = nullptr;

DirectDevice::~DirectDevice()
{
	m_SwapChain->SetFullscreenState(false, nullptr);
}

void DirectDevice::SetWindow(HWND window, int width, int height)
{
	m_hWnd = window;
	m_ScreenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<float>(width),
		static_cast<float>(height)
	);
}

HRESULT DirectDevice::CreateDevice()
{
	HRESULT hr = S_OK;
	
	const D3D_DRIVER_TYPE driverTypes[] =
	{
		D3D_DRIVER_TYPE_HARDWARE,
		D3D_DRIVER_TYPE_WARP,
		D3D_DRIVER_TYPE_REFERENCE,
	};
	UINT numDriverTypes = ARRAYSIZE(driverTypes);

	const D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1,
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	// 上位の機能レベルから順にデバイスが作成できるかを試す
	for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++) {
		m_DriverType = driverTypes[driverTypeIndex];
		hr = D3D11CreateDevice(
			nullptr,
			m_DriverType,
			nullptr,
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			m_d3dDevice.ReleaseAndGetAddressOf(),
			&m_FeatureLevel,
			m_ImmediateContext.ReleaseAndGetAddressOf()
		);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
		return hr;

	IDXGIFactory2* dxgiFactory = nullptr;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
	if (FAILED(hr))
		return hr;

	// スワップチェーンの設定
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.Width  = static_cast<UINT>(m_ScreenViewport.Width);
	swapChainDesc.Height = static_cast<UINT>(m_ScreenViewport.Height);
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = 2;
	swapChainDesc.SampleDesc.Count = 1;
	swapChainDesc.SampleDesc.Quality = 0;
	swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
	swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

	DXGI_SWAP_CHAIN_FULLSCREEN_DESC fsSwapChainDesc = {};
	fsSwapChainDesc.Windowed = TRUE;

	// スワップチェーンの作成
	hr = dxgiFactory->CreateSwapChainForHwnd(
		m_d3dDevice.Get(),
		m_hWnd,
		&swapChainDesc,
		&fsSwapChainDesc,
		nullptr, 
		m_SwapChain.ReleaseAndGetAddressOf()
	);
	dxgiFactory->Release();
	if (FAILED(hr))
		return hr;
	
	// レンダーターゲットビューの作成
	hr = CreateRenderTargetView();
	if (FAILED(hr))
		return hr;

	// 深度ステンシルビューの作成
	hr = CreateDepthStencilView();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DirectDevice::CreateRenderTargetView()
{
	HRESULT hr = S_OK;

	// バックバッファの取得
	ID3D11Texture2D* backBuffer = nullptr;
	hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
	if (FAILED(hr))
		return hr;

	// レンダーターゲットビューの作成
	hr = m_d3dDevice->CreateRenderTargetView(backBuffer, nullptr, m_RenderTargetView.ReleaseAndGetAddressOf());
	backBuffer->Release();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DirectDevice::CreateDepthStencilView()
{
	HRESULT hr = S_OK;

	//深度ステンシルバッファの作成
	D3D11_TEXTURE2D_DESC depthDesc = {};
	depthDesc.Width = static_cast<UINT>(m_ScreenViewport.Width);
	depthDesc.Height = static_cast<UINT>(m_ScreenViewport.Height);
	depthDesc.MipLevels = 1;
	depthDesc.ArraySize = 1;
	depthDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthDesc.SampleDesc.Count = 1;
	depthDesc.SampleDesc.Quality = 0;
	depthDesc.Usage = D3D11_USAGE_DEFAULT;
	depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	depthDesc.CPUAccessFlags = 0;
	depthDesc.MiscFlags = 0;
	hr = m_d3dDevice->CreateTexture2D(&depthDesc, nullptr, m_DepthStencil.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	//深度ステンシルビューの作成
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd = {};
	dsvd.Format = depthDesc.Format;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	dsvd.Texture2D.MipSlice = 0;
	hr = m_d3dDevice->CreateDepthStencilView(m_DepthStencil.Get(), &dsvd,
		m_DepthStencilView.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DirectDevice::ResizeSwapChain(int width, int height)
{
	// スワップチェーンへの参照を解放する
	m_ImmediateContext->OMSetRenderTargets(0, 0, 0);
	SAFE_RELEASE(m_DepthStencilView);
	SAFE_RELEASE(m_DepthStencil);
	SAFE_RELEASE(m_RenderTargetView);

	// スワップチェーンのリサイズ
	HRESULT hr = m_SwapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING);
	if (FAILED(hr))
		return hr;

	// ビューポートの更新
	m_ScreenViewport = CD3D11_VIEWPORT(
		0.0f,
		0.0f,
		static_cast<float>(width),
		static_cast<float>(height)
	);

	// レンダーターゲットビューの再作成
	hr = CreateRenderTargetView();
	if (FAILED(hr))
		return hr;

	// 深度ステンシルビューの再作成
	hr = CreateDepthStencilView();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void DirectDevice::StartRendering()
{
	float clearColor[4] = { 0.7f, 0.7f, 1.0f, 0.0f };
	m_ImmediateContext->ClearRenderTargetView(m_RenderTargetView.Get(), clearColor);
	m_ImmediateContext->ClearDepthStencilView(m_DepthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	m_ImmediateContext->OMSetRenderTargets(1, m_RenderTargetView.GetAddressOf(), m_DepthStencilView.Get());
	m_ImmediateContext->RSSetViewports(1, &m_ScreenViewport);
}

void DirectDevice::FinishRendering()
{
	HRESULT hr = m_SwapChain->Present(1, 0);
}