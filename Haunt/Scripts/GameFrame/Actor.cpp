#include "Actor.h"

Actor::Actor(LPCWSTR name)
	: m_Status(EActorStatus::ACTIVE)
{
	wcscpy_s(m_ActorName, name);
}

Actor::~Actor()
{
	// �q��S�ĉ������
	for (auto it = m_Children.begin(); it != m_Children.end();) {
		(*it).reset();
		it = m_Children.erase(it);
	}
}

HRESULT Actor::CreateObject()
{
	for (auto& child : m_Children) {
		if (child->m_Status != EActorStatus::DEAD)
			if (FAILED(child->CreateObject()))
				return E_FAIL;
	}

	return S_OK;
}

void Actor::FrameMove(const double& time, const float& fElapsedTime)
{
	for (auto& child : m_Children) {
		if (child->m_Status == EActorStatus::ACTIVE ||
			child->m_Status == EActorStatus::MOVEONLY)
			child->FrameMove(time, fElapsedTime);
	}
}
void Actor::FrameRender(const double& time, const float& fElapsedTime)
{
	for (auto& child : m_Children) {
		if (child->m_Status == EActorStatus::ACTIVE ||
			child->m_Status == EActorStatus::RENDERONLY)
			child->FrameRender(time, fElapsedTime);
	}
}

void Actor::OnResizedSwapChain()
{
	for (auto& child : m_Children) {
		if (child->m_Status != EActorStatus::DEAD)
			child->OnResizedSwapChain();
	}
}

void Actor::AddChild(std::unique_ptr<Actor> actor)
{
	m_Children.push_back(std::move(actor));
}

Actor* Actor::Search(const LPCWSTR name)
{
	// �����ł���΁A������Ԃ�
	if (wcscmp(name, m_ActorName) == 0)
		return this;

	// �q���ċA�I�Ɍ������Ă���
	Actor* retActor = nullptr;
	for (auto it = m_Children.begin(); it != m_Children.end();) {
		retActor = (*it)->Search(name);
		if (retActor != nullptr)
			return retActor;
		it++;
	}
	return nullptr;
}

void Actor::CheckStatus()
{
	// ���������S���Ă�����A�S�Ă̎q�Ɏ��S�t���O�𗧂Ă�
	if (this->m_Status == EActorStatus::DEAD) {
		for (auto& child : m_Children) {
			child->m_Status = EActorStatus::DEAD;
		}
	}

	// �q��CheckStatus���ċA�I�Ɏ��s���Ă���
	for (auto& child : m_Children) {
		child->CheckStatus();
	}

	// ���S�t���O�̗����Ă���q�����X�g����폜
	for (auto it = m_Children.begin(); it != m_Children.end();) {
		if ((*it)->m_Status == DEAD) {
			(*it).reset();
			it = m_Children.erase(it);
		}
		else {
			it++;
		}
	}
}

void Actor::SetStatus(const EActorStatus status)
{
	m_Status = status;
}

const std::list <std::unique_ptr<Actor>>* Actor::GetChildren()
{
	return &m_Children;
}

EActorStatus Actor::GetStatus()
{
	return m_Status;
}

WCHAR* Actor::GetName()
{
	return m_ActorName;
}