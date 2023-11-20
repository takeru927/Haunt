#pragma once
#include "ModelActor.h"
#include "GameLibrary/Shader.h"

using namespace DirectX;

class Scene : public Actor
{
protected:
	Shader* m_Shader;

public:
	Scene(LPCWSTR name, Shader* const shader) : Actor(name), m_Shader(shader)
	{ }

	virtual ~Scene() = default;

	virtual XMMATRIX GetCameraView()
	{
		return XMMatrixIdentity();
	}
	virtual XMMATRIX GetCameraLocation()
	{
		return XMMatrixIdentity();
	}

	ModelActor* AddSceneObject(const LPCWSTR fileName, const XMVECTOR location, const XMVECTOR rotation, const XMVECTOR scale)
	{
		auto object = std::make_unique<ModelActor>(L"SceneObject", m_Shader, fileName);
		object->SetLocation(location);
		object->SetRotation(rotation);
		object->SetScale(scale);

		auto res = object.get();
		this->AddChild(std::move(object));

		return res;
	}
};