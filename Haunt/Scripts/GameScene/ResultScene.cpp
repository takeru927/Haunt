#include "ResultScene.h"
#include "GameMain.h"
#include "GameFrame/TextActor.h"
#include "GameManager/InputManager.h"
#include "GameManager/XAudio2Manager.h"
#include "GameLibrary/DirectDevice.h"
#include "GameActor/Fade.h"

ResultScene::ResultScene(Shader* const shader)
	: Scene(L"ResultScene", shader), m_isFade(false), m_Timer(0.0f)
{
	auto clear = std::make_unique<SpriteActor>(L"GameClear");
	auto viewport = D3D->GetScreenViewport();
	clear->Initialize(viewport.Width, viewport.Height, 0.1f, true);
	clear->CreateTextureResourse(L"Contents/Textures/GameClear.png");

	auto fade = std::make_unique<Fade>();

	this->AddChild(std::move(clear));
	this->AddChild(std::move(fade));

	XAudio2Manager::GetInstance()->OpenWave(L"Clear", L"Contents/Sounds/GameClear.wav");
}

HRESULT ResultScene::CreateObject()
{
	XMFLOAT4 lightDirection = { 0.0f, 1.0f, 2.0f, 0.0f };
	XMFLOAT4 lightAmbinet = { 1.0f, 1.0f, 1.0f, 0.0f };
	XMFLOAT4 lightDiffuse = { 1.0f, 1.0f, 1.0f, 0.0f };
	m_Shader->SetDirectionalLight(lightDirection, lightAmbinet, lightDiffuse);

	dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeIn(2.0f);

	if (FAILED(Scene::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void ResultScene::FrameMove(const double& time, const float& fElapsedTime)
{
	auto input = InputManager::GetInstance();
	
	if (m_isFade == false && input->GetKeyReleased("Enter")) {
		dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeOut(1.2f);
		XAudio2Manager::GetInstance()->Play(L"Enter");
		m_isFade = true;
	}

	if (m_isFade)
	{
		m_Timer += fElapsedTime;
		if (m_Timer >= 1.2f) {
			this->SetStatus(DEAD);
			GAME->OpenScene(EScene::TITLE);
		}
	}

	Scene::FrameMove(time, fElapsedTime);
}

void ResultScene::FrameRender(const double& time, const float& fElapsedTime)
{
	Scene::FrameRender(time, fElapsedTime);
}

void ResultScene::SetStatus(const EActorStatus status)
{
	if (status == EActorStatus::ACTIVE)
	{
		XAudio2Manager::GetInstance()->StopBGM();
		XAudio2Manager::GetInstance()->Play(L"Clear");
	}

	Scene::SetStatus(status);
}