#pragma once

enum EActorStatus {
	ACTIVE,		// �s����
	MOVEONLY,	// Move�̂ݍs��
	RENDERONLY, // Render�̂ݍs��
	NOUSE,		// ���ݖ��g�p
	DEAD,		// ���S���
};

class Actor
{
protected:
	 WCHAR m_ActorName[128];

	 EActorStatus	m_Status;
	 std::list <std::unique_ptr<Actor>> m_Children;

public:
	Actor(LPCWSTR name);
	virtual ~Actor();

	virtual HRESULT CreateObject();
	virtual void FrameMove(const double& time, const float& fElapsedTime);
	virtual void FrameRender(const double& time, const float& fElapsedTime);

	virtual void OnResizedSwapChain();

	void AddChild(std::unique_ptr<Actor> actor);
	Actor* Search(const LPCWSTR name);
	void CheckStatus();
	virtual void SetStatus(const EActorStatus status);
	
	const std::list <std::unique_ptr<Actor>>* GetChildren();
	EActorStatus GetStatus();
	WCHAR* GetName();
};