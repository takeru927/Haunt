#include <Windows.h>
#include "resource.h"

#include "GameMain.h"
#include "GameLibrary/DirectDevice.h"
#include "GameLibrary/DirectWrite.h"
#include "GameLibrary/StepTimer.h"

//----------------------------------------------------------------
// デフォルトウィンドウサイズ
//----------------------------------------------------------------
constexpr int WINDOW_WIDTH  = 1280;
constexpr int WINDOW_HEIGHT = 720;

//----------------------------------------------------------------
// グローバル変数
//----------------------------------------------------------------
HINSTANCE	  g_hInst = nullptr;
HWND		  g_hWnd  = nullptr;
DX::StepTimer g_Timer;
const int	  g_FPS = 60;

//----------------------------------------------------------------
// 前方宣言
//----------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
HRESULT OnResizedWindow();

HRESULT InitDevice();
void ReleaseDevice();
void ExitGame();

//----------------------------------------------------------------
// 高性能GPUを使用する
//----------------------------------------------------------------
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}

//----------------------------------------------------------------
// プログラムのエントリーポイント
//----------------------------------------------------------------
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	if (FAILED(hr))
		return 1;

	hr = InitWindow(hInstance, nCmdShow);
	if (FAILED(hr))
		return 1;

	hr = InitDevice();
	if (FAILED(hr)) {
		ReleaseDevice();
		return 1;
	}

	GameMain::CreateInstance();
	hr = GAME->Initialize(g_FPS);
	if (FAILED(hr)) {
		ReleaseDevice();
		return 1;
	}

	MSG msg = { };
	while (WM_QUIT != msg.message) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			double time = g_Timer.GetTotalSeconds();
			float elapsedTime = static_cast<float>(g_Timer.GetElapsedSeconds());

			g_Timer.Tick([&]()
				{
					GAME->FrameMove(time, elapsedTime);
				});
			
			D3D->StartRendering();
			GAME->FrameRender(time, elapsedTime);
			D3D->FinishRendering();
		}
	}

	ReleaseDevice();

	CoUninitialize();

	return static_cast<int>(msg.wParam);
}

//----------------------------------------------------------------
// ウィンドウの作成
//----------------------------------------------------------------
HRESULT InitWindow(HINSTANCE hInstance, int nCmdShow)
{
	UNREFERENCED_PARAMETER(nCmdShow);

	// ウィンドウの設定
	WNDCLASSEX wcex = { };
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	wcex.hCursor		= LoadCursor(hInstance, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName	= nullptr;
	wcex.lpszClassName	= L"HauntWindowClass";
	wcex.hIconSm		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1));
	if (!RegisterClassEx(&wcex))
		return E_FAIL;

	// ウィンドウの作成
	g_hInst = hInstance;
	RECT rc = { 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	g_hWnd = CreateWindowEx(0, L"HauntWindowClass", L"Haunt", WS_POPUP,
		CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, nullptr, nullptr, hInstance, nullptr);
	if (!g_hWnd)
		return E_FAIL;

	ShowWindow(g_hWnd, SW_SHOWMAXIMIZED);

	return S_OK;
}

//----------------------------------------------------------------
// ウィンドウからのメッセージを取得する度に実行される
//----------------------------------------------------------------
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static bool isFullscreen = true;

	switch (message)
	{
	case WM_GETMINMAXINFO:
		if (lParam) {
			auto info = reinterpret_cast<MINMAXINFO*>(lParam);
			info->ptMinTrackSize.x = 320;
			info->ptMinTrackSize.y = 200;
		}
		break;

	case WM_SIZE:
		OnResizedWindow();
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_SYSKEYDOWN:
		// ALT+ENTER でフルスクリーン切り替え
		if (wParam == VK_RETURN && (lParam & 0x60000000) == 0x20000000)
		{
			isFullscreen = !isFullscreen;

			if (isFullscreen)
			{
				// ウィンドウスタイルの変更
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);

				D3D->GetSwapChain()->SetFullscreenState(true, nullptr);
				ShowWindow(hWnd, SW_SHOWMAXIMIZED);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			}
			else
			{
				// ウィンドウスタイルの変更
				SetWindowLongPtr(hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);

				// アイコンの設定
				HICON hIcon = LoadIcon(g_hInst, MAKEINTRESOURCE(IDI_ICON1));
				SendMessage(hWnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);

				D3D->GetSwapChain()->SetFullscreenState(false, nullptr);
				ShowWindow(hWnd, SW_SHOWNORMAL);
				SetWindowPos(hWnd, HWND_TOP, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, SWP_NOMOVE | SWP_FRAMECHANGED);
			}
		}
		break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

//----------------------------------------------------------------
// ウィンドウのリサイズ時に呼び出す
//----------------------------------------------------------------
HRESULT OnResizedWindow()
{
	HRESULT hr = S_OK;
	
	if (D3D == nullptr || DWRITE == nullptr|| GAME == nullptr)
		return E_FAIL;

	DWRITE->ReleaseRT();

	RECT rc = {};
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	hr = D3D->ResizeSwapChain(width, height);
	if (FAILED(hr))
		return hr;

	hr = DWRITE->OnResizedSwapChain(D3D->GetSwapChain(), 96.0f);
	if (FAILED(hr))
		return hr;

	GAME->OnResizedSwapChain();

	return S_OK;
}

//----------------------------------------------------------------
// DirectXの初期化
//----------------------------------------------------------------
HRESULT InitDevice()
{
	HRESULT hr = S_OK;

	// インスタンスの作成
	DirectDevice::CreateInstance();
	DirectWrite::CreateInstance();

	RECT rc = {};
	GetClientRect(g_hWnd, &rc);
	UINT width = rc.right - rc.left;
	UINT height = rc.bottom - rc.top;

	D3D->SetWindow(g_hWnd, width, height);
	hr = D3D->CreateDevice();
	if (FAILED(hr))
		return hr;

	hr = DWRITE->CreateDevice(D3D->GetSwapChain(), 96.0f);
	if(FAILED(hr))
		return hr;

	g_Timer.SetFixedTimeStep(true);
	g_Timer.SetTargetElapsedSeconds(1.0 / g_FPS);

	return S_OK;
}

void ReleaseDevice()
{
	GameMain::DeleteInstance();
	DirectWrite::DeleteInstance();
	DirectDevice::DeleteInstance();
}

void ExitGame()
{
	PostQuitMessage(0);
}