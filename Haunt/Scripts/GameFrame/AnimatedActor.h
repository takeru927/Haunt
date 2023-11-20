#pragma once
#include "SceneActor.h"
#include "GameLibrary/AnimationShader.h"
#include "GameManager/SkeltalMesh.h"
#include "GameManager/Animation.h"

class AnimatedActor : public SceneActor
{
protected:
	WCHAR						 m_FileName[256];
	SkeltalMesh*				 m_Model;
	AnimationShader*			 m_AnimShader;
	std::unique_ptr<Animation[]> m_Animation;
	UINT						 m_AnimState;

public:
	AnimatedActor(LPCWSTR name, AnimationShader* shader, LPCWSTR fileName);
	virtual ~AnimatedActor() = default;

	virtual HRESULT CreateObject();
	virtual void FrameMove(const double& time, const float& fElapsedTime);
	virtual void FrameRender(const double& time, const float& fElapsedTime);

	virtual HRESULT LoadAnimation() = 0;

	void UpdateAnimation(const double& time, const float& fElapsedTime);
	void ChangeAnimState(const UINT state);
};