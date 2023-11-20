#include "CutScene.h"
#include "GameFrame/SceneActor.h"
#include "GameFrame/AnimatedActor.h"
#include "GameFrame/Scene.h"

CutScene::CutScene(Scene* const scene)
	: m_Scene(scene), m_On(false), m_Span(0.0f), m_Time(0.0f), m_CurrentFrame(0), m_EndFrame(0)
{

}

void CutScene::CreateCameraCut(std::vector<CameraKey>& keys)
{
	// 線形補間
	m_CameraCut.resize((size_t)keys.back().KeyFrame + 1);
	for (size_t i = 0; i < keys.size(); i++)
	{
		m_CameraCut[keys[i].KeyFrame] = keys[i];

		if (keys[i].KeyFrame == 0)
			continue;

		int frameCount = keys[i].KeyFrame - keys[i - 1].KeyFrame;
		XMVECTOR eyeLerp = (keys[i].Eye - keys[i - 1].Eye) / (float)frameCount;
		XMVECTOR atLerp = (keys[i].At - keys[i - 1].At) / (float)frameCount;

		for (size_t j = 1; j < frameCount; j++)
		{
			m_CameraCut[keys[i - 1].KeyFrame + j].KeyFrame = keys[i - 1].KeyFrame + (int)j;
			m_CameraCut[keys[i - 1].KeyFrame + j].Eye = m_CameraCut[keys[i - 1].KeyFrame + j - 1].Eye + eyeLerp;
			m_CameraCut[keys[i - 1].KeyFrame + j].At = m_CameraCut[keys[i - 1].KeyFrame + j - 1].At + atLerp;
		}
	}
}

void CutScene::CreateActorCut(SceneActor* actor, bool hasAnim, std::vector<ActorKey>& keys)
{
	std::vector<ActorKey> cutKeys;

	// 線形補間
	cutKeys.resize((size_t)keys.back().KeyFrame + 1);
	for (size_t i = 0; i < keys.size(); i++)
	{
		cutKeys[keys[i].KeyFrame] = keys[i];
		if (keys[i].KeyFrame == 0)
			continue;

		int frameCount		  = keys[i].KeyFrame - keys[i - 1].KeyFrame;
		XMVECTOR locationLerp = (keys[i].Location - keys[i - 1].Location) / (float)frameCount;
		XMVECTOR rotationLerp = (keys[i].Rotation - keys[i - 1].Rotation) / (float)frameCount;
		XMVECTOR scaleLerp	  = (keys[i].Scale - keys[i - 1].Scale) / (float)frameCount;

		for (size_t j = 1; j < frameCount; j++)
		{
			cutKeys[keys[i - 1].KeyFrame + j].KeyFrame  = keys[i - 1].KeyFrame + (int)j;
			cutKeys[keys[i - 1].KeyFrame + j].Location  = cutKeys[keys[i - 1].KeyFrame + j - 1].Location + locationLerp;
			cutKeys[keys[i - 1].KeyFrame + j].Rotation  = cutKeys[keys[i - 1].KeyFrame + j - 1].Rotation + rotationLerp;
			cutKeys[keys[i - 1].KeyFrame + j].Scale     = cutKeys[keys[i - 1].KeyFrame + j - 1].Scale + scaleLerp;
			cutKeys[keys[i - 1].KeyFrame + j].AnimState = keys[i - 1].AnimState;
		}
	}

	m_ActorCut.emplace_back(ActorKeyInfo{ actor, hasAnim, cutKeys });
}

void CutScene::CreateEventCut(int keyFrame, std::function<void()> func)
{
	m_EventCut.emplace_back(EventKey{ keyFrame, func });
}

void CutScene::Play(const float spanTime)
{
	m_On			= true;
	m_Span			= spanTime;
	m_Time			= 0.0f;
	m_CurrentFrame	= 0;
	m_EndFrame		= (int)m_CameraCut.size() - 1;
}

void CutScene::Update(const double& time, const float& fElapsedTime)
{
	if (m_On == false)
		return;

	// 開始
	if (m_CurrentFrame == 0) {
		auto children = m_Scene->GetChildren();
		for (auto& actor : *children) {
			actor->SetStatus(RENDERONLY);
		}
	}

	// カットシーンに関係なくアニメーションは更新する
	for (auto& actorCut : m_ActorCut) {
		if (m_CurrentFrame >= actorCut.CutKeys.size())
			continue;

		if (actorCut.HasAnimation) {
			auto animActor = dynamic_cast<AnimatedActor*>(actorCut.pActor);
			animActor->UpdateAnimation(time, fElapsedTime);
			animActor->ChangeAnimState((UINT)actorCut.CutKeys[m_CurrentFrame].AnimState);
		}
	}

	m_Time += fElapsedTime;
	if (m_Time > m_Span)
	{
		m_Time -= m_Span;

		for (auto& actorCut : m_ActorCut) {
			if (m_CurrentFrame >= actorCut.CutKeys.size())
				continue;

			actorCut.pActor->SetLocation(actorCut.CutKeys[m_CurrentFrame].Location);
			actorCut.pActor->SetRotation(actorCut.CutKeys[m_CurrentFrame].Rotation);
			actorCut.pActor->SetScale(actorCut.CutKeys[m_CurrentFrame].Scale);
		}

		for (auto& eventCut : m_EventCut) {
			if (eventCut.KeyFrame == m_CurrentFrame)
				eventCut.Event();
		}

		// 終了
		if (m_CurrentFrame++ >= m_EndFrame) {
			m_On = false;

			auto children = m_Scene->GetChildren();
			for (auto& actor : *children) {
				actor->SetStatus(ACTIVE);
			}
		}
	}
}

bool CutScene::IsPlaying()
{
	return m_On;
}

bool CutScene::IsEnd()
{
	return m_CurrentFrame > m_EndFrame;
}

int CutScene::GetCurrentFrame()
{
	return m_CurrentFrame;
}

XMMATRIX CutScene::GetCamaraView()
{
	XMVECTOR up = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMVECTOR eye = m_CameraCut[m_CurrentFrame].Eye;
	XMVECTOR at = m_CameraCut[m_CurrentFrame].At;
	XMMATRIX view = XMMatrixLookAtLH(eye, at, up);

	return view;
}

XMMATRIX CutScene::GetCamaraLocation()
{
	XMMATRIX location = XMMatrixTranslationFromVector(m_CameraCut[m_CurrentFrame].Eye);

	return location;
}