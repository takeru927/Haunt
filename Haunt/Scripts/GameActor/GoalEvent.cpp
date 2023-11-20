#include "GoalEvent.h"
#include "GameManager/Collision.h"
#include "GameMain.h"

GoalEvent::GoalEvent(CutScene* cut)
	: SceneActor(L"GoalEvent"), m_GoalCut(cut)
{
	m_Status = MOVEONLY;
}

HRESULT GoalEvent::CreateObject()
{
	auto scale = XMMatrixScalingFromVector(m_Scale);
	auto rotation = XMMatrixRotationRollPitchYawFromVector(m_Rotation);
	auto location = XMMatrixTranslationFromVector(m_Location);
	m_World = scale * rotation * location;

	this->AddBoxCollision(INFLICT, { 0.0f, 23.0f, 0.0f }, { 3.0f, 25.0f, 32.0f }, true);

	if (FAILED(Actor::CreateObject()))
		return E_FAIL;

	return S_OK;
}
void GoalEvent::FrameMove(const double& time, const float& fElapsedTime)
{
	SceneActor::FrameMove(time, fElapsedTime);
}

void GoalEvent::SetStatus(const EActorStatus status)
{
	// 描画はしない
	if (status == ACTIVE) {
		Actor::SetStatus(MOVEONLY);
		return;
	}
	else if (status == RENDERONLY) {
		Actor::SetStatus(NOUSE);
		return;
	}

	Actor::SetStatus(status);
}

void GoalEvent::OnCollide(SceneActor* other)
{
	if (wcscmp(other->GetName(), L"Player") == 0)
	{
		size_t count = GAME->GetRootActor()->Search(L"CoinManager")->GetChildren()->size();

		// カットシーンの再生
		if (count <= 0) {
			m_GoalCut->Play();
			m_Status = NOUSE;
		}
	}
}