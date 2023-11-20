#include "GameMain.h"
#include "GameLibrary/DirectDevice.h"

#include "GameManager/InputManager.h"
#include "GameManager/XAudio2Manager.h"
#include "GameManager/MeshManager.h"
#include "GameScene/TitleScene.h"
#include "GameScene/MainScene.h"
#include "GameScene/ResultScene.h"
#include "GameScene/GameOverScene.h"

using namespace DirectX;

GameMain* GameMain::instance = nullptr;

extern void ExitGame();

GameMain::~GameMain()
{
	m_RootActor.reset();
	m_CollisionManager.reset();

	MeshManager::DeleteInstance();
	XAudio2Manager::DeleteInstance();
	InputManager::DeleteInstance();
}

HRESULT GameMain::Initialize(int fps)
{
	HRESULT hr = S_OK;

	m_FPS = fps;

	// インスタンスの作成
	InputManager::CreateInstance();
	XAudio2Manager::CreateInstance();
	MeshManager::CreateInstance();

	m_Shader = std::make_unique<Shader>();
	hr = m_Shader->CreateShader();
	if (FAILED(hr)) 
		return hr;

	m_AnimShader = std::make_unique<AnimationShader>();
	hr = m_AnimShader->CreateShader();
	if (FAILED(hr))
		return hr;

	InputManager::GetInstance()->Initialize();
	
	m_RootActor = std::make_unique<Actor>(L"Root");

	m_CollisionManager = std::make_unique<CollisionManager>();
	
	auto title = std::make_unique<TitleScene>(m_Shader.get());
	title->SetStatus(ACTIVE);
	m_pCurrentScene = title.get();

	m_RootActor->AddChild(std::move(title));

	XAudio2Manager::GetInstance()->OpenWave(L"Enter", L"Contents/Sounds/Enter.wav");

	hr = m_RootActor->CreateObject();
	if (FAILED(hr))
		return hr;

	OnResizedSwapChain();

	return S_OK;
}

void GameMain::FrameMove(const double& time, const float& fElapsedTime)
{
	InputManager::GetInstance()->Update();
	XAudio2Manager::GetInstance()->Update();

	m_RootActor->FrameMove(time, fElapsedTime);
	m_RootActor->CheckStatus();

	m_CollisionManager->Update();
	m_CollisionManager->CalculateCollision();

	// シーン遷移
	if (m_NextScene != NONE)
	{
		switch (m_NextScene)
		{
		case TITLE:
			{
				auto title = std::make_unique<TitleScene>(m_Shader.get());
				title->SetStatus(ACTIVE);
				m_pCurrentScene = title.get();

				m_RootActor->AddChild(std::move(title));
				break;
			}

		case MAIN:
			{
				auto main = std::make_unique<MainScene>(m_Shader.get(), m_AnimShader.get());
				main->SetStatus(ACTIVE);
				m_pCurrentScene = main.get();

				m_RootActor->AddChild(std::move(main));
				break;
			}

		case RESULT:
			{
				auto result = std::make_unique<ResultScene>(m_Shader.get());
				result->SetStatus(ACTIVE);
				m_pCurrentScene = result.get();

				m_RootActor->AddChild(std::move(result));
				break;
			}

		case GAMEOVER:
			{
				auto over = std::make_unique<GameOverScene>(m_Shader.get());
				over->SetStatus(ACTIVE);
				m_pCurrentScene = over.get();

				m_RootActor->AddChild(std::move(over));
				break;
			}
		}

		MeshManager::GetInstance()->ReleaseUnusedMesh();
		m_RootActor->CreateObject();
 		this->OnResizedSwapChain();
		m_NextScene = NONE;
	}
}

void GameMain::FrameRender(const double& time, const float& fElapsedTime)
{
	m_RootActor->FrameRender(time, fElapsedTime);
}

void GameMain::QuitGame()
{
	ExitGame();
}

void GameMain::OnResizedSwapChain()
{
	// 射影行列の設定
	auto screen = D3D->GetScreenViewport();
	auto aspect = screen.Width / screen.Height;
	auto projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 0.01f, 1000.0f);
	m_Shader->SetProjection(projection);
	m_AnimShader->SetProjection(projection);

	m_RootActor->OnResizedSwapChain();
}

void GameMain::OpenScene(EScene nextScene)
{
	m_NextScene = nextScene;
}

XMMATRIX GameMain::GetCameraView()
{
	auto view = m_pCurrentScene->GetCameraView();
	return view;
}

XMMATRIX GameMain::GetCameraLocation()
{
	auto location = m_pCurrentScene->GetCameraLocation();
	return location;
}