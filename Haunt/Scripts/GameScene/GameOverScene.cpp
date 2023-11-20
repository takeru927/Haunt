#include "GameOverScene.h"
#include "GameMain.h"
#include "GameFrame/TextActor.h"
#include "GameManager/InputManager.h"
#include "GameManager/XAudio2Manager.h"
#include "GameLibrary/DirectDevice.h"

#include "GameFrame/SpriteActor.h"
#include "GameActor/Fade.h"

GameOverScene::GameOverScene(Shader* const shader)
	: Scene(L"GameOverScene", shader), m_isFade(false), m_Timer(0.0f)
{
	auto over = std::make_unique<SpriteActor>(L"GameOver");
	auto viewport = D3D->GetScreenViewport();
	over->Initialize(viewport.Width, viewport.Height, 0.1f, true);
	over->CreateTextureResourse(L"Contents/Textures/GameOver.png");

	auto fade = std::make_unique<Fade>();

	this->AddChild(std::move(over));
	this->AddChild(std::move(fade));
}

HRESULT GameOverScene::CreateObject()
{
	dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeIn(1.5f);

	if (FAILED(Scene::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void GameOverScene::FrameMove(const double& time, const float& fElapsedTime)
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

void GameOverScene::FrameRender(const double& time, const float& fElapsedTime)
{
	Scene::FrameRender(time, fElapsedTime);
}

void GameOverScene::SetStatus(const EActorStatus status)
{
	if (status == EActorStatus::ACTIVE)
	{
		XAudio2Manager::GetInstance()->StopBGM();
	}

	Scene::SetStatus(status);
}