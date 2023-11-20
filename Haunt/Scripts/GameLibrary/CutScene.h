#pragma once

using namespace DirectX;
class SceneActor;
class Scene;

struct CameraKey
{
	int		 KeyFrame;
	XMVECTOR Eye;
	XMVECTOR At;
};

struct ActorKey
{
	int		 KeyFrame;
	XMVECTOR Location;
	XMVECTOR Rotation;
	XMVECTOR Scale;
	UINT	 AnimState;
};

struct EventKey
{
	int KeyFrame;
	std::function<void()> Event;
};

struct ActorKeyInfo
{
	SceneActor*			  pActor;
	bool				  HasAnimation;
	std::vector<ActorKey> CutKeys;
};

class CutScene
{
private:
	Scene* m_Scene;

	bool  m_On;
	float m_Span;
	float m_Time;
	int	  m_CurrentFrame;
	int	  m_EndFrame;
	
	std::vector<CameraKey>	   m_CameraCut;
	std::vector<ActorKeyInfo>  m_ActorCut;
	std::vector<EventKey>	   m_EventCut;

public:
	CutScene(Scene* const scene);
	~CutScene() = default;

	void CreateCameraCut(std::vector<CameraKey>& keys);
	void CreateActorCut(SceneActor* actor, bool hasAnim, std::vector<ActorKey>& keys);
	void CreateEventCut(int keyFrame, std::function<void()> func);
	void Play(const float spanTime = 0.033333f);
	void Update(const double& time, const float& fElapsedTime);

	bool IsPlaying();
	bool IsEnd();
	int GetCurrentFrame();

	XMMATRIX GetCamaraView();
	XMMATRIX GetCamaraLocation();
};