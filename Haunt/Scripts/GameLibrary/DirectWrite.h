#pragma once
#include <d2d1.h>
#include <dwrite.h>

using Microsoft::WRL::ComPtr;

#define DWRITE DirectWrite::GetInstance()

class DirectWrite
{
//--------------------------------------------------
// シングルトンパターンコード
//--------------------------------------------------
private:
	DirectWrite()
		: m_d2dFactory(nullptr)
		, m_WriteFactory(nullptr)
		, m_RenderTarget(nullptr)
	{
	}

	static DirectWrite* instance;

public:
	static void CreateInstance()
	{
		instance = new DirectWrite();
	}

	static void DeleteInstance()
	{
		delete instance;
	}

	static DirectWrite* GetInstance()
	{
		return instance;
	}

private:
	ComPtr<ID2D1Factory>		 m_d2dFactory;
	ComPtr<IDWriteFactory>		 m_WriteFactory;
	ComPtr<ID2D1RenderTarget>	 m_RenderTarget;

public:
	~DirectWrite() = default;

	HRESULT CreateDevice(IDXGISwapChain* swapChain, float dpi);
	HRESULT CreateRenderTarget(IDXGISwapChain* swapChain, float dpi);
	void ReleaseRT();

	HRESULT OnResizedSwapChain(IDXGISwapChain* swapChain, float dpi);

	IDWriteFactory* GetWriteFactory() const	{ return m_WriteFactory.Get(); }
	ID2D1RenderTarget* GetRenderTarget() const { return m_RenderTarget.Get(); }
};