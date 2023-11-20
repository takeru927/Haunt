#pragma once
#include "GameLibrary/Shader.h"
#include "GameLibrary/AnimationShader.h"
#include "GameFrame/Actor.h"
#include "GameFrame/Scene.h"
#include "GameManager/CollisionManager.h"

#define GAME GameMain::GetInstance()

enum EScene{
	NONE,
	TITLE,
	MAIN,
	RESULT,
	GAMEOVER,
};

// シングルトンクラス
class GameMain
{
private:
	GameMain()
		: m_Shader(nullptr), m_AnimShader(nullptr)
		, m_RootActor(nullptr), m_CollisionManager(nullptr)
		, m_FPS(60), m_pCurrentScene(nullptr), m_NextScene(EScene::NONE)
	{ }
	
	static GameMain* instance;

public:
	static void CreateInstance()
	{
		instance = new GameMain();
	}

	static void DeleteInstance()
	{
		delete instance;
	}

	static GameMain* GetInstance()
	{
		return instance;
	}

private:
	std::unique_ptr<Shader>			  m_Shader;
	std::unique_ptr<AnimationShader>  m_AnimShader;
	std::unique_ptr<Actor>			  m_RootActor;
	std::unique_ptr<CollisionManager> m_CollisionManager;

	int	   m_FPS;
	Scene* m_pCurrentScene;
	EScene m_NextScene;

public:
	~GameMain();

	HRESULT Initialize(int fps);
	void FrameMove(const double& time, const float& fElapsedTime);
	void FrameRender(const double& time, const float& fElapsedTime);
	void QuitGame();

	void OnResizedSwapChain();
	void OpenScene(EScene nextScene);
	DirectX::XMMATRIX GetCameraView();
	DirectX::XMMATRIX GetCameraLocation();

	Actor* GetRootActor() const					  { return m_RootActor.get(); }
	CollisionManager* GetCollisionManager() const { return m_CollisionManager.get(); }
	int GetFPS() const							  { return m_FPS; }
};