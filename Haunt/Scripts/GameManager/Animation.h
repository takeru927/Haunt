#pragma once
#include <unordered_map>

namespace fbxsdk {
	class FbxMesh;
}

using namespace DirectX;

__declspec(align(16))
struct Bone {
	CHAR				  Name[256];
	XMMATRIX			  InvInitMat;
	std::vector<XMMATRIX> FrameMat;

	Bone() {
		strcpy_s(Name, "");
		InvInitMat = XMMatrixIdentity();
	}

	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(Bone));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};

__declspec(align(16))
class Animation
{
private:
	float m_Time;
	float m_Span;
	UINT  m_CurrentFrame;
	UINT  m_StartFrame;
	UINT  m_EndFrame;

	std::vector<Bone>			m_BoneTransforms;
	std::unique_ptr<XMMATRIX[]> m_FrameBones;

public:
	Animation();
	~Animation() = default;

	HRESULT LoadAnimation(LPCWSTR fileName, const std::unordered_map<std::string, UINT>& bones);
	void Update(const double& time, const float& fElapsedTime);
	void ResetAnimation();

	XMMATRIX* GetFrameBones();

private:
	HRESULT GetMeshBones(fbxsdk::FbxMesh* mesh, const std::unordered_map<std::string, UINT>& bones);

public:
	void* operator new(size_t size) {
		return _mm_malloc(size, alignof(Animation));
	}
	void operator delete(void* p) {
		return _mm_free(p);
	}
};