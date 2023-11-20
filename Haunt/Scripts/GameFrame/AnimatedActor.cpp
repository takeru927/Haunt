#include "AnimatedActor.h"
#include "GameManager/MeshManager.h"

AnimatedActor::AnimatedActor(LPCWSTR name, AnimationShader* shader, LPCWSTR fileName)
	: SceneActor(name), m_Model(nullptr), m_AnimShader(shader), m_Animation(nullptr), m_AnimState(0)
{
	wcscpy_s(m_FileName, fileName);
	MeshManager::GetInstance()->ReserveCreateMesh(fileName);
}

HRESULT AnimatedActor::CreateObject()
{
	HRESULT hr = S_OK;

	// マネージャーからモデルデータを取得
	m_Model = MeshManager::GetInstance()->GetSkeltalMeshObject(m_FileName);
	if (m_Model == nullptr)
		return E_FAIL;

	hr = LoadAnimation();
	if (FAILED(hr))
		return hr;

	hr = SceneActor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void AnimatedActor::FrameMove(const double& time, const float& fElapsedTime)
{
	// アニメーションの更新
	this->UpdateAnimation(time, fElapsedTime);

	Actor::FrameMove(time, fElapsedTime);
}

void AnimatedActor::FrameRender(const double& time, const float& fElapsedTime)
{
	// ワールド行列の計算
	auto scale = XMMatrixScalingFromVector(m_Scale);
	auto rotation = XMMatrixRotationRollPitchYawFromVector(m_Rotation);
	auto location = XMMatrixTranslationFromVector(m_Location);
	m_World = scale * rotation * location;
	m_AnimShader->SetWorld(m_World);

	// ボーンの設定
	m_AnimShader->SetBones(m_Animation[m_AnimState].GetFrameBones());

	// 描画
	m_Model->RenderModel([&](const Material* material) {
		// シェーダの設定
		m_AnimShader->SetMaterial(material->Ambient, material->Diffuse, material->Texture);
		m_AnimShader->SetRenderShader();
	});

	Actor::FrameRender(time, fElapsedTime);
}

void AnimatedActor::UpdateAnimation(const double& time, const float& fElapsedTime)
{
	m_Animation[m_AnimState].Update(time, fElapsedTime);
}

void AnimatedActor::ChangeAnimState(const UINT state)
{
	if (m_AnimState != state)
	{
		m_AnimState = state;
		m_Animation[m_AnimState].ResetAnimation();
	}
}