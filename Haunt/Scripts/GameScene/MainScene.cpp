#include "MainScene.h"
#include "GameMain.h"
#include "GameManager/XAudio2Manager.h"
#include "GameManager/InputManager.h"

#include "GameActor/EnemySimple.h"
#include "GameActor/CoinManager.h"
#include "GameActor/GoalEvent.h"
#include "GameActor/RemainingText.h"
#include "GameActor/TimeLimit.h"
#include "GameActor/EffectManager.h"
#include "GameActor/SkyBox.h"
#include "GameActor/Fade.h"

namespace {
	constexpr float ChaseStartDistance = 70.0f;
	constexpr float ChaseMinVolume = 0.2f;
	constexpr float ChaseMaxVolume = ChaseStartDistance / (ChaseStartDistance * 0.5f) + ChaseMinVolume;
}

MainScene::MainScene(Shader* const shader, AnimationShader* const animShader)
	: Scene(L"MainScene", shader), m_AnimShader(animShader), m_Player(nullptr)
	, m_Init(false), m_PlayingChaseBGM(false), m_Timer(0.0f)
{
	m_GoalCut = std::make_unique<CutScene>(this);

	LoadMap();

	// シーンにアクターを追加
	auto player = std::make_unique<Player>(m_AnimShader);
	player->InitCamera(0, XM_PIDIV4, 15.0f);
	player->SetLocation({ 0.0f, 0.0f, 0.0f, 1.0f });
	player->SetRotation({ 0.0f, -1.57f, 0.0f, 1.0f });
	player->SetScale({ 0.025f, 0.025f, 0.025f, 1.0f });
	m_Player = player.get();

	auto enemy = std::make_unique<EnemySimple>(m_Shader);
	enemy->SetLocation({ -200.0f, -4.0f, 0.0f, 1.0f });
	enemy->SetScale({ 2.5f, 2.5f, 2.5f, 1.0f });

	auto goal = std::make_unique<GoalEvent>(m_GoalCut.get());
	goal->SetLocation({ -490.0f, 0.0f, 0.0f, 1.0f });

	auto itemManager = std::make_unique<CoinManager>(m_Shader);
	auto remain = std::make_unique<RemainingText>();
	auto time = std::make_unique<TimeLimit>();
	auto effectManager = std::make_unique<EffectManager>();
	auto fade = std::make_unique<Fade>();

	this->AddChild(std::move(player));
	this->AddChild(std::move(enemy));
	this->AddChild(std::move(goal));
	this->AddChild(std::move(itemManager));
	this->AddChild(std::move(remain));
	this->AddChild(std::move(time));
	this->AddChild(std::move(effectManager));
	this->AddChild(std::move(fade));

	XAudio2Manager::GetInstance()->OpenWave(L"Coin", L"Contents/Sounds/GetCoin.wav");
	XAudio2Manager::GetInstance()->OpenWave(L"OpenGate", L"Contents/Sounds/GateOpen.wav");
	XAudio2Manager::GetInstance()->OpenWave(L"MainBGM", L"Contents/Sounds/MainBGM.wav");
	XAudio2Manager::GetInstance()->OpenWave(L"Chase", L"Contents/Sounds/Chase.wav");
}

HRESULT MainScene::CreateObject()
{
	XMFLOAT4 lightDirection = { 1.0f, 1.0f, 2.0f, 0.0f };
	XMFLOAT4 lightAmbinet = { 0.3f, 0.3f, 0.3f, 0.0f };
	XMFLOAT4 lightDiffuse = { 1.0f, 0.6f, 0.6f, 0.0f };
	m_Shader->SetDirectionalLight(lightDirection, lightAmbinet, lightDiffuse);
	m_AnimShader->SetDirectionalLight(lightDirection, lightAmbinet, lightDiffuse);

	LoadCutScene();

	dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeIn(1.0f);

	if (FAILED(Scene::CreateObject()))
		return E_FAIL;

	return S_OK;
}

void MainScene::FrameMove(const double& time, const float& fElapsedTime)
{
	auto audio = XAudio2Manager::GetInstance();

	if (m_Init == false) {
		audio->PlayBGM(L"MainBGM", 0.8f, 1.0f);
		m_Init = true;
	}

	// ゴールカットシーン
	if (m_GoalCut->IsPlaying())
	{
		m_GoalCut->Update(time, fElapsedTime);

		if (m_GoalCut->IsEnd()) {
			this->SetStatus(DEAD);
			GAME->OpenScene(EScene::RESULT);
		}
	}

	// ゲームオーバー
	else if (m_Player->IsDead()) {
		m_Timer += fElapsedTime;

		if (m_Timer >= 1.2f) {
			this->SetStatus(DEAD);
			GAME->OpenScene(EScene::GAMEOVER);
		}
	}

	// タイトルシーンへ戻る
	else if (InputManager::GetInstance()->GetKeyReleased("Esc")) {
		this->SetStatus(DEAD);
		GAME->OpenScene(EScene::TITLE);
	}

	// 追跡BGMの再生
	else {
		XMVECTOR player = m_Player->GetLocation();
		XMVECTOR enemy = dynamic_cast<ModelActor*>(this->Search(L"Enemy"))->GetLocation();
		float distance = XMVectorGetX(XMVector3Length(player - enemy));

		if (distance <= ChaseStartDistance) {
			if (m_PlayingChaseBGM == false) {
				audio->PlayBGM(L"Chase");
				m_PlayingChaseBGM = true;
			}
			float volume = ChaseMaxVolume - distance / (ChaseStartDistance * 0.5f);
			audio->SetBGMVolume(volume);
		}
		else {
			if (m_PlayingChaseBGM == true) {
				audio->PlayBGM(L"MainBGM", 0.8f, 2.0f);
				m_PlayingChaseBGM = false;
			}
		}
	}

	Scene::FrameMove(time, fElapsedTime);
}

void MainScene::FrameRender(const double& time, const float& fElapsedTime)
{
	auto view = this->GetCameraView();
	m_Shader->SetView(view);
	m_AnimShader->SetView(view);

	Scene::FrameRender(time, fElapsedTime);
}

void MainScene::SetStatus(const EActorStatus status)
{
	if (status == EActorStatus::ACTIVE) {
		m_Init = false;
	}

	Scene::SetStatus(status);
}

XMMATRIX MainScene::GetCameraView()
{
	if (m_GoalCut->IsPlaying())
		return m_GoalCut->GetCamaraView();
	else
		return m_Player->GetCameraView();
}

XMMATRIX MainScene::GetCameraLocation()
{
	if (m_GoalCut->IsPlaying())
		return m_GoalCut->GetCamaraLocation();
	else
		return m_Player->GetCameraLocation();
}

void MainScene::LoadCutScene()
{
	// カメラキーフレーム
	std::vector<CameraKey> camera = {
		{   0, { -470.0f, 30.0f, 0.0f, 0.0f },{ -500.0f, 15.0f, 0.0f, 0.0f }},
		{ 130, { -470.0f, 30.0f, 0.0f, 0.0f },{ -500.0f, 15.0f, 0.0f, 0.0f }},
		{ 200, { -470.0f, 30.0f, 0.0f, 0.0f },{ -500.0f, 30.0f, 0.0f, 0.0f }},
	};
	m_GoalCut->CreateCameraCut(camera);

	// アクターキーフレーム
	std::vector<ActorKey> player = {
		{   0, { -490.0f, 0.0f, 0.0f },{ 0.0f,-1.57f,0.0f }, { 0.025f, 0.025f, 0.025f, 1.0f }, EAnimPlayer::IDLE },
		{  80, { -490.0f, 0.0f, 0.0f },{ 0.0f,-1.57f,0.0f }, { 0.025f, 0.025f, 0.025f, 1.0f }, EAnimPlayer::RUN  },
		{ 200, { -550.0f, 0.0f, 0.0f },{ 0.0f,-1.57f,0.0f }, { 0.025f, 0.025f, 0.025f, 1.0f }, EAnimPlayer::RUN  },
	};
	m_GoalCut->CreateActorCut(static_cast<SceneActor*>(Search(L"Player")), true, player);

	std::vector<ActorKey> latticeLeft = {
		{  0, { -495.0f, 0.0f, -36.0f },{ 0.0f,1.57f,0.0f }, { 0.8f,0.7f,0.7f,1.0f } },
		{ 15, { -495.0f, 0.0f, -36.0f },{ 0.0f,1.57f,0.0f }, { 0.8f,0.7f,0.7f,1.0f } },
		{ 70, { -495.0f, 0.0f, -36.0f },{ 0.0f, 0.0f,0.0f }, { 0.8f,0.7f,0.7f,1.0f } },
	};
	m_GoalCut->CreateActorCut(static_cast<SceneActor*>(Search(L"LatticeLeft")), false, latticeLeft);

	std::vector<ActorKey> latticeRight = {
		{  0, { -495.0f, 0.0f, 36.0f },{ 0.0f,-1.57f,0.0f }, { 0.8f,0.7f,0.7f,1.0f } },
		{ 15, { -495.0f, 0.0f, 36.0f },{ 0.0f,-1.57f,0.0f }, { 0.8f,0.7f,0.7f,1.0f } },
		{ 70, { -495.0f, 0.0f, 36.0f },{ 0.0f, 0.0f,0.0f }, { 0.8f,0.7f,0.7f,1.0f } },
	};
	m_GoalCut->CreateActorCut(static_cast<SceneActor*>(Search(L"LatticeRight")), false, latticeRight);

	// イベントキーフレーム
	m_GoalCut->CreateEventCut(170, [&] {
		dynamic_cast<Fade*>(this->Search(L"Fade"))->FadeOut(1.0f);
		XAudio2Manager::GetInstance()->StopBGM(1.0f);
		});

	m_GoalCut->CreateEventCut(15, [&] {
		XAudio2Manager::GetInstance()->Play(L"OpenGate");
		});
}

void MainScene::LoadMap()
{
	auto skybox = std::make_unique<SkyBox>(L"Contents/Textures/Skybox");
	this->AddChild(std::move(skybox));
	
	auto ground1 = std::make_unique<ModelActor>(L"Ground", m_Shader, L"Contents/Models/Ground.obj");
	ground1->SetScale({ 10.0f, 1.0f, 10.0f, 1.0f });
	ground1->AddBoxCollision(OBJECT, { 0.0f, -0.5f }, { 500.0f,1.0f,500.0f });
	this->AddChild(std::move(ground1));

	auto ground2 = std::make_unique<ModelActor>(L"Ground", m_Shader, L"Contents/Models/Ground.obj");
	ground2->SetScale({ 10.0f, 1.0f, 10.0f, 1.0f });
	ground2->SetLocation({ -1000.0f, 0.0f, 0.0f, 1.0f });
	ground2->AddBoxCollision(OBJECT, { 0.0f, -0.5f }, { 500.0f,1.0f,500.0f });
	this->AddChild(std::move(ground2));

	auto latticeLeft = std::make_unique<ModelActor>(L"LatticeLeft", m_Shader, L"Contents/Models/GateLattice.obj");
	latticeLeft->SetLocation({ -495.0f, 0.0f, -36.0f });
	latticeLeft->SetRotation({ 0.0f,1.57f,0.0f });
	latticeLeft->SetScale({ 0.8f,0.7f,0.7f,1.0f });
	latticeLeft->AddBoxCollision(OBJECT, { -20.0f, 23.0f, 0.0f }, { 17.0f, 25.0f, 2.0f });
	this->AddChild(std::move(latticeLeft));
	auto latticeRight = std::make_unique<ModelActor>(L"LatticeRight", m_Shader, L"Contents/Models/GateLattice.obj");
	latticeRight->SetLocation({ -495.0f, 0.0f, 36.0f });
	latticeRight->SetRotation({ 0.0f,-1.57f,0.0f });
	latticeRight->SetScale({ 0.8f,0.7f,0.7f,1.0f });
	latticeRight->AddBoxCollision(OBJECT, { -20.0f, 23.0f, 0.0f }, { 17.0f, 25.0f, 2.0f });
	this->AddChild(std::move(latticeRight));

	ModelActor* object;
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 70.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 70.0f,0.0f,70.3f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 70.0f,0.0f,-70.3f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 70.0f,0.0f,-140.6f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 30.0f,0.0f,-200.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -40.3f,0.0f,-200.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 140.3f,0.0f,70.3f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -40.3f,0.0f,-270.3f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 30.0f,0.0f,-270.3f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -130.0f,0.0f,-460.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -200.3f,0.0f,-460.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 280.0f,0.0f,100.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 280.0f,0.0f,29.7f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 310.0f,0.0f,170.3f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 310.0f,0.0f,240.6f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 380.0f,0.0f,110.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 380.0f,0.0f,39.7f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 460.0f,0.0f,-80.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 389.7f,0.0f,-80.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 319.4f,0.0f,-80.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 360.0f,0.0f,-150.3f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 360.0f,0.0f,-220.6f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 460.0f,0.0f,-398.7f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 398.0f,0.0f,-460.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 398.0f,0.0f,-398.7f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 280.0f,0.0f,-460.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 209.7f,0.0f,-460.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 139.4f,0.0f,-460.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 280.0f,0.0f,-375.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 209.7f,0.0f,-375.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 139.4f,0.0f,-375.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 350.0f,0.0f,350.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 420.3f,0.0f,350.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -5.0f,0.0f,460.0f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -5.0f,0.0f,389.7f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -75.3f,0.0f,460.0f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -145.6f,0.0f,460.0f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -215.9f,0.0f,460.0f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -260.9f,0.0f,389.7f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -331.2f,0.0f,389.7f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -460.0f,0.0f,460.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -460.0f,0.0f,389.3f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -290.0f,0.0f,280.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -290.0f,0.0f,209.7f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 460.0f,0.0f,220.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 0.0f,0.0f,180.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 70.3f,0.0f,180.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -70.3f,0.0f,180.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 190.0f,0.0f,280.0f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 119.7f,0.0f,280.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { 49.4f,0.0f,280.0f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -180.0f,0.0f,70.0f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -180.0f,0.0f,140.3f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -250.3f,0.0f,70.3f }, { 0.0f,1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -320.6f,0.0f,70.3f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -180.0f,0.0f,-70.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -180.0f,0.0f,-140.3f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -250.3f,0.0f,-70.3f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -320.6f,0.0f,-70.3f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -310.0f,0.0f,-420.0f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -310.0f,0.0f,-349.7f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -310.0f,0.0f,-279.4f }, { 0.0f,3.14f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -310.0f,0.0f,-209.1f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -380.3f,0.0f,-209.1f }, { 0.0f,-1.57f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -460.0f,0.0f,-460.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -460.0f,0.0f,-389.7f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 37.0f, 25.0f, 37.0f });

	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -495.0f,0.0f,-100.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -495.0f,0.0f,-220.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -495.0f,0.0f,-340.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -495.0f,0.0f,100.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -495.0f,0.0f,220.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -495.0f,0.0f,340.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 440.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 320.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 200.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 80.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -40.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -160.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -280.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -400.0f,0.0f,495.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,440.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,320.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,200.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,80.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,-40.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,-160.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,-280.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 495.0f,0.0f,-400.0f }, { 0.0f,-1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 440.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 320.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 200.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { 80.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -40.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -160.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -280.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -400.0f,0.0f,-495.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 60.0f, 25.0f, 1.0f });

	object = AddSceneObject(L"Contents/Models/House00.obj", { -60.0f,0.0f,-60.0f }, { 0.0f,2.0f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { 180.0f,0.0f,180.0f }, { 0.0f,-1.0f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { 390.0f,0.0f,419.0f }, { 0.0f,1.57f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { 290.0f,0.0f,-180.0f }, { 0.0f,0.0f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { 432.0f,0.0f,-250.0f }, { 0.0f,-1.7f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { -70.0f,0.0f,290.0f }, { 0.0f,0.53f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { -190.0f,0.0f,260.0f }, { 0.0f,2.83f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { -135.0f,0.0f,365.0f }, { 0.0f,-1.57f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { 65.0f,0.0f,-450.0f }, { 0.0f,1.34f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { -25.0f,0.0f,-450.0f }, { 0.0f,1.9f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { 25.0f,0.0f,-340.0f }, { 0.0f,-1.6f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { -55.0f,0.0f,-350.0f }, { 0.0f,-1.8f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });
	object = AddSceneObject(L"Contents/Models/House00.obj", { -390.0f,0.0f,-285.0f }, { 0.0f,-0.3f,0.0f }, { 9.0f,9.0f,9.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 5.5f, 15.0f, -3.5f }, { 26.0f,22.0f,37.0f });
	object->AddBoxCollision(OBJECT, { -25.0f, 7.5f, -3.5f }, { 5.0f,10.0f, 6.5f });

	object = AddSceneObject(L"Contents/Models/House01.obj", { -55.0f,0.0f,65.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { 170.0f,0.0f,-50.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { 170.0f,0.0f,-150.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { 170.0f,0.0f,-250.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { 442.0f,0.0f,-172.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { 220.0f,0.0f,400.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { 115.0f,0.0f,400.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { -180.0f,0.0f,-350.0f }, { 0.0f,1.57f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { -180.0f,0.0f,-255.0f }, { 0.0f,1.57f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });
	object = AddSceneObject(L"Contents/Models/House01.obj", { -410.0f,0.0f,250.0f }, { 0.0f,1.57f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 3.0f, 16.0f, 0.0f }, { 36.0f,20.0f,45.0f });

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 425.0f,3.5f,389.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 425.0f,6.5f,389.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 321.0f,3.5f,-145.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 321.0f,6.5f,-145.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 397.0f,3.5f,-259.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 397.0f,6.5f,-259.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 413.0f,3.5f,-219.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 413.0f,9.5f,-219.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 99.0f,3.5f,-435.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 99.0f,6.5f,-435.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -10.0f,3.5f,-353.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { -10.0f,6.5f,-353.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -411.0f,3.5f,-248.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { -411.0f,6.5f,-248.5f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -411.0f,3.5f,-254.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { -411.0f,6.5f,-253.5f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { -411.0f,12.5f,-251.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -349.0f,3.5f,-317.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -349.0f,9.5f,-317.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 348.0f,3.5f,390.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 348.0f,3.5f,396.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 348.0f,6.5f,390.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 348.0f,6.5f,396.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 322.0f,3.5f,-222.5f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 316.0f,3.5f,-222.5f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 322.0f,6.5f,-222.5f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 316.0f,6.5f,-222.5f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 67.0f,0.5f,-307.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 67.0f,0.5f,-312.0f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);
	object = AddSceneObject(L"Contents/Models/WoodBarrel.obj", { 67.0f,6.5f,-309.5f }, { 0.0f,0.0f,0.0f }, { 5.0f,5.0f,5.0f,1.0f });
	object->AddCapsuleCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, 3.5f, 3.0f);

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -5.0f,3.5f,-491.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -5.0f,9.5f,-491.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -92.0f,3.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -92.0f,9.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -86.0f,3.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -86.0f,9.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -86.0f,15.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -80.0f,3.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -80.0f,9.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -80.0f,15.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -80.0f,21.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -74.0f,3.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -74.0f,9.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -74.0f,15.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -68.0f,3.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -68.0f,9.5f,-442.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -27.0f,3.5f,-309.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { -27.0f,9.5f,-309.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });

	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 203.0f,3.5f,-97.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 203.0f,9.5f,-97.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 203.0f,15.5f,-97.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 203.0f,3.5f,-103.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 203.0f,9.5f,-103.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });
	object = AddSceneObject(L"Contents/Models/WoodBox.obj", { 203.0f,15.5f,-103.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 2.0f, 0.0f }, { 4.0f, 4.0f, 4.0f });

	object = AddSceneObject(L"Contents/Models/GatePillar.obj", { -495.0f,0.0f,-37.0f }, { 0.0f,1.57f,0.0f }, { 0.8f,0.7f,0.7f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 7.0f, 25.0f, 7.0f });
	object = AddSceneObject(L"Contents/Models/GatePillar.obj", { -495.0f,0.0f, 37.0f }, { 0.0f,1.57f,0.0f }, { 0.8f,0.7f,0.7f,1.0f });
	object->AddBoxCollision(OBJECT, { 0.0f, 23.0f, 0.0f }, { 7.0f, 25.0f, 7.0f });

	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -555.0f,0.0f,40.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -660.0f,0.0f,100.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -720.0f,0.0f,40.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -840.0f,0.0f,40.0f }, { 0.0f,3.14f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -555.0f,0.0f,-40.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -660.0f,0.0f,-100.0f }, { 0.0f,1.57f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -720.0f,0.0f,-40.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/OutsideWall.obj", { -840.0f,0.0f,-40.0f }, { 0.0f,0.0f,0.0f }, { 10.0f,10.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -950.0f,0.0f,0.0f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -950.0f,0.0f,70.4f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
	object = AddSceneObject(L"Contents/Models/HouseWall.obj", { -950.0f,0.0f,-70.4f }, { 0.0f,0.0f,0.0f }, { 6.0f,6.0f,6.0f,1.0f });
}