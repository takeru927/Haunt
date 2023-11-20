#pragma once
#include "SceneActor.h"
#include "GameManager/StaticMesh.h"
#include "GameLibrary/Shader.h"

__declspec(align(16))
class ModelActor : public SceneActor
{
protected:
	WCHAR		 m_FileName[256];
	StaticMesh*  m_Model;
	Shader*		 m_Shader;

public:
	ModelActor(LPCWSTR name, Shader* shader, LPCWSTR fileName);
	virtual ~ModelActor() = default;

	virtual HRESULT CreateObject();
	virtual void FrameMove(const double& time, const float& fElapsedTime);
	virtual void FrameRender(const double& time, const float& fElapsedTime);

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(ModelActor));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};