#pragma once

using Microsoft::WRL::ComPtr;

#define D3D DirectDevice::GetInstance()

// シングルトンクラス
class DirectDevice
{
private:
	DirectDevice()
		: m_DriverType(D3D_DRIVER_TYPE_NULL)
		, m_FeatureLevel(D3D_FEATURE_LEVEL_9_1)
		, m_d3dDevice(nullptr)
		, m_ImmediateContext(nullptr)
		, m_SwapChain(nullptr)
		, m_RenderTargetView(nullptr)
		, m_DepthStencil(nullptr)
		, m_DepthStencilView(nullptr)
		, m_hWnd(nullptr)
		, m_ScreenViewport{ 0.0f, 0.0f, 1.0f, 1.0f }
	{ }

	static DirectDevice* instance;

public:
	static void CreateInstance()
	{
		instance = new DirectDevice();
	}

	static void DeleteInstance()
	{
		delete instance;
	}

	static DirectDevice* GetInstance()
	{
		return instance;
	}

private:
	D3D_DRIVER_TYPE		m_DriverType;
	D3D_FEATURE_LEVEL	m_FeatureLevel;

	ComPtr<ID3D11Device>			m_d3dDevice;
	ComPtr<ID3D11DeviceContext>		m_ImmediateContext;
	ComPtr<IDXGISwapChain1>			m_SwapChain;
	ComPtr<ID3D11RenderTargetView>	m_RenderTargetView;
	ComPtr<ID3D11Texture2D>			m_DepthStencil;
	ComPtr<ID3D11DepthStencilView>	m_DepthStencilView;

	HWND			m_hWnd;
	D3D11_VIEWPORT	m_ScreenViewport;

public:
	~DirectDevice();

	void SetWindow(HWND window, int width, int height);
	HRESULT CreateDevice();
	HRESULT CreateRenderTargetView();
	HRESULT CreateDepthStencilView();

	HRESULT ResizeSwapChain(int width, int height);

	void StartRendering();
	void FinishRendering();

	ID3D11Device*	     GetDevice()		 const { return m_d3dDevice.Get(); }
	ID3D11DeviceContext* GetDeviceContext()  const { return m_ImmediateContext.Get(); }
	IDXGISwapChain1*		 GetSwapChain()		 const { return m_SwapChain.Get(); }
	D3D11_VIEWPORT		 GetScreenViewport() const { return m_ScreenViewport; }
};