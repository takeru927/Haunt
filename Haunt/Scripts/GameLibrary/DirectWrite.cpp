#include "DirectWrite.h"

#define SAFE_RELEASE(p) { if(p) {(p).Reset(); (p) = nullptr; } }
DirectWrite* DirectWrite::instance = nullptr;

HRESULT DirectWrite::CreateDevice(IDXGISwapChain* swapChain, float dpi)
{
	HRESULT hr = S_OK;

	// Direct2Dの作成
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, m_d2dFactory.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	hr = CreateRenderTarget(swapChain, dpi);
	if (FAILED(hr))
		return hr;

	// DirectWriteの作成
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
		reinterpret_cast<IUnknown**>(m_WriteFactory.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT DirectWrite::CreateRenderTarget(IDXGISwapChain* swapChain, float dpi)
{
	HRESULT hr = S_OK;

	// バックバッファの取得
	ComPtr<IDXGISurface> backBuffer = nullptr;
	hr = swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.ReleaseAndGetAddressOf()));
	if (FAILED(hr))
		return hr;

	// レンダーターゲットの設定
	D2D1_RENDER_TARGET_PROPERTIES props = D2D1::RenderTargetProperties(
		D2D1_RENDER_TARGET_TYPE_DEFAULT,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		dpi, dpi);

	// レンダーターゲットの作成
	hr = m_d2dFactory->CreateDxgiSurfaceRenderTarget(backBuffer.Get(), &props, m_RenderTarget.ReleaseAndGetAddressOf());
	backBuffer.Reset();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void DirectWrite::ReleaseRT()
{
	SAFE_RELEASE(m_RenderTarget);
}

HRESULT DirectWrite::OnResizedSwapChain(IDXGISwapChain* swapChain, float dpi)
{
	HRESULT hr = S_OK;

	// レンダーターゲットの再作成
	hr = CreateRenderTarget(swapChain, dpi);
	if (FAILED(hr))
		return hr;

	return S_OK;
}