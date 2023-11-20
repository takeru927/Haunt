#pragma once

#include "GameFrame/Actor.h"
#include "GameLibrary/EffectShader.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

namespace
{
	// エフェクト情報
	__declspec(align(16))
	struct EffectNode
	{
		bool	 m_On;			// 再生中か？
		XMVECTOR m_Location;	// 表示位置
		int		 m_Texture;		// テクスチャ番号
		float	 m_Time;		// 経過時間
		float	 m_Span;		// 1枚の表示時間

		void* operator new(size_t size) {
			return _mm_malloc(size, alignof(EffectNode));
		}
		void operator delete(void* p) {
			return _mm_free(p);
		}
	};
}

__declspec(align(16))
class Effect : public Actor
{
private:
	WCHAR							 m_FileName[128];
	std::unique_ptr<EffectShader>	 m_Shader;
	ComPtr<ID3D11ShaderResourceView> m_TextureRV;
	ComPtr<ID3D11Buffer>			 m_VertexBuffer;

	float m_Scale;
	int	  m_SizeX;
	int	  m_SizeY;
	int	  m_Size;

	int	  m_NumNode;
	std::vector<std::unique_ptr<EffectNode>> m_Node;

public:
	Effect(LPCWSTR fileName, const int num = 1);
	~Effect() = default;

	void Initialize(const float scale = 1.0f, const int sizeX = 1, const int sizeY = 1);
	HRESULT CreateObject() override;

	HRESULT PlayEffect(CXMVECTOR location, const float spanTime = 0.033333f, const float startTime = 0.0f);

	void FrameMove(const double& time, const float& fElapsedTime) override;
	void FrameRender(const double& time, const float& fElapsedTime) override;

	void OnResizedSwapChain() override;

private:
	HRESULT CreateVertex();
};