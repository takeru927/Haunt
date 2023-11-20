#include "ModelActor.h"
#include "GameManager/Collision.h"
#include "GameManager/MeshManager.h"

using namespace DirectX;

ModelActor::ModelActor(LPCWSTR name, Shader* shader, LPCWSTR fileName)
	: SceneActor(name), m_Model(nullptr), m_Shader(shader)
{
	wcscpy_s(m_FileName, fileName);
	MeshManager::GetInstance()->ReserveCreateMesh(fileName);
}

HRESULT ModelActor::CreateObject()
{
	HRESULT hr = S_OK;

	// �}�l�[�W���[���烂�f���f�[�^���擾
	m_Model = MeshManager::GetInstance()->GetStaticMeshObject(m_FileName);
	if (m_Model == nullptr)
		return E_FAIL;

	hr = SceneActor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void ModelActor::FrameMove(const double& time, const float& fElapsedTime)
{
	SceneActor::FrameMove(time, fElapsedTime);
}

void ModelActor::FrameRender(const double& time, const float& fElapsedTime)
{
	// ���[���h�s��̌v�Z
	auto scale = XMMatrixScalingFromVector(m_Scale);
	auto rotation = XMMatrixRotationRollPitchYawFromVector(m_Rotation);
	auto location = XMMatrixTranslationFromVector(m_Location);
	m_World = scale * rotation * location;

	// 3D���f���̕`��
	m_Shader->SetWorld(m_World);
	m_Model->RenderModel([&](const Material* material) {
		// �V�F�[�_�̐ݒ�
		m_Shader->SetMaterial(material->Ambient, material->Diffuse, material->Texture);
		m_Shader->SetRenderShader();
		});

	SceneActor::FrameRender(time, fElapsedTime);
}