#include "TitleScene.h"
#include "GameMain.h"
#include "GameFrame/TextActor.h"
#include "GameManager/InputManager.h"
#include "GameManager/XAudio2Manager.h"
#include "GameLibrary/DirectDevice.h"

#include "GameActor/Fade.h"
#include "GameActor/TitleLogo.h"
#include "GameActor/TitleGuideBlink.h"

TitleScene::TitleScene(Shader* const shader)
	: Scene(L"TitleScene", shader)
{
	m_StartCut = std::make_unique<CutScene>(this);

	LoadMap();

	auto logo = std::make_unique<TitleLogo>();
	auto guide = std::make_unique<TitleGuideBlink>(0.6f, 0.4f);
	auto fade = std::make_unique<Fade>();

	this->AddChild(std::move(logo));
	this->AddChild(std::move(guide));
	this->AddChild(std::move(fade));

	XAudio2Manager::GetInstance()->OpenWave(L"TitleBGM", L"Contents/Sounds/TitleSound.wav");
}

HRESULT TitleScene::CreateObject()
{
	XMFLOAT4 lightDirection = { 0.0f, 1.0f, 2.0f, 0.0f };
	XMFLOAT4 lightAmbinet = { 0.1f, 0.1f, 0.1f, 0.0f };
	XMFLOAT4 lightDiffuse = { 1.0f, 0.6f, 0.6f, 0.0f };
	m_Shader->SetDirectionalLight(lightDirection, lightAmbinet, lightDiffuse);

	LoadCutScene();

	dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeIn(1.5f);

	if (FAILED(Scene::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void TitleScene::FrameMove(const double& time, const float& fElapsedTime)
{
	// スタート時カットシーンの再生
	if (m_StartCut->IsPlaying())
	{
		m_StartCut->Update(time, fElapsedTime);

		// メインシーンへ進む
		if(m_StartCut->IsEnd())
		{
			this->SetStatus(DEAD);
			GAME->OpenScene(EScene::MAIN);
		}
	}
	else
	{
		// スタート
		if (InputManager::GetInstance()->GetKeyReleased("Enter")) {
			XAudio2Manager::GetInstance()->Play(L"Enter");
			m_StartCut->Play();
			dynamic_cast<TitleLogo*>(this->Search(L"TitleLogo"))->FadeOut(0.4f);
			this->Search(L"TitleGuideBlink")->SetStatus(DEAD);
		}

		// ゲームを終了する
		else if (InputManager::GetInstance()->GetKeyReleased("Esc")) {
			GAME->QuitGame();
		}
	}

	Scene::FrameMove(time, fElapsedTime);
}

void TitleScene::FrameRender(const double& time, const float& fElapsedTime)
{
	XMMATRIX view = this->GetCameraView();
	m_Shader->SetView(view);

	Scene::FrameRender(time, fElapsedTime);
}

void TitleScene::SetStatus(const EActorStatus status)
{
	if (status == EActorStatus::ACTIVE)
	{
		XAudio2Manager::GetInstance()->PlayBGM(L"TitleBGM", 1.0f, 2.0f);
	}

	Scene::SetStatus(status);
}

XMMATRIX TitleScene::GetCameraView()
{
	XMMATRIX view = {};

	if (m_StartCut->IsPlaying())
		view = m_StartCut->GetCamaraView();

	else {
		XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
		XMVECTOR eye = { 0.0f, 20.0f, 10.0f, 0.0f };
		XMVECTOR at = { 0.0f, 5.0f, -30.0f, 0.0f };
		view = XMMatrixLookAtLH(eye, at, up);
	}

	return view;
}

XMMATRIX TitleScene::GetCameraLocation()
{
	XMMATRIX location = {};

	if (m_StartCut->IsPlaying())
		location = m_StartCut->GetCamaraLocation();

	else {
		XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
		XMVECTOR eye = { 0.0f, 20.0f, 10.0f, 0.0f };
		XMVECTOR at = { 0.0f, 5.0f, -30.0f, 0.0f };
		location = XMMatrixLookAtLH(eye, at, up);
	}

	return location;
}

void TitleScene::LoadCutScene()
{
	// カメラキーフレーム
	std::vector<CameraKey> camera = {
		{  0, { 0.0f, 20.0f, 10.0f, 0.0f },{ 0.0f, 5.0f, -30.0f, 0.0f }},
		{ 10, { 0.0f, 17.0f,  8.0f, 0.0f },{ 0.0f, 5.0f, -30.0f, 0.0f }},
		{ 20, { 0.0f, 13.0f,  5.5f, 0.0f },{ 0.0f, 5.0f, -30.0f, 0.0f }},
		{ 35, { 0.0f,  9.0f,  0.0f, 0.0f },{ 0.0f, 5.0f, -30.0f, 0.0f }},
		{ 50, { 0.0f,  7.8f, -7.0f, 0.0f },{ 0.0f, 5.0f, -30.0f, 0.0f }},
		{ 70, { 0.0f, 7.0f, -15.0f, 0.0f },{ 0.0f, 5.0f, -30.0f, 0.0f }},
	};
	m_StartCut->CreateCameraCut(camera);

	// イベントキーフレーム
	m_StartCut->CreateEventCut(40, [&]() {
		dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeOut(1.0f);
		XAudio2Manager::GetInstance()->StopBGM(1.0f);
		});
}

void TitleScene::LoadMap()
{
	auto ground = std::make_unique<ModelActor>(L"Ground", m_Shader, L"Contents/Models/Ground.obj");
	ground->SetScale({ 2.0f, 1.0f, 1.0f, 1.0f });
	this->AddChild(std::move(ground));

	AddSceneObject(L"Contents/Models/HouseWall.obj", {  0.0f,0.0f,-50.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	AddSceneObject(L"Contents/Models/HouseWall.obj", { 70.3f,0.0f,-50.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	AddSceneObject(L"Contents/Models/HouseWall.obj", {-70.3f,0.0f,-50.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
}