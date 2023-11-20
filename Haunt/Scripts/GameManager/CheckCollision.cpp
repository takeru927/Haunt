#include "CheckCollision.h"
#include "GameMain.h"
#include "GameLibrary/DirectDevice.h"
#include "Collision.h"

namespace
{
	struct SimpleVertex {
		XMFLOAT3 Pos;
	};

	// カプセル描画用変数
	constexpr int uMax = 24;	// 横の分割数
	constexpr int vMax = 12;	// 縦の分割数
	constexpr int vertexNum = uMax * (vMax + 1) + uMax;
	constexpr int indexNum = 2 * vMax * (uMax + 1) + 2 * (uMax + 1);
}

CheckCollision::CheckCollision(Collision* const collision)
	: Actor(L"CheckCollision")
	, m_CollisionShader(nullptr), m_Collision(collision)
	, m_VertexBuffer(nullptr), m_IndexBuffer(nullptr), m_PrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST)
	, m_IndexNum(0)
{
}

HRESULT CheckCollision::CreateObject()
{
	HRESULT hr = S_OK;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	// シェーダの作成
	m_CollisionShader = std::make_unique<CollisionShader>();
	hr = m_CollisionShader->CreateShader();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

void CheckCollision::FrameMove(const double& time, const float& fElapsedTime)
{
	Actor::FrameMove(time, fElapsedTime);
}

void CheckCollision::FrameRender(const double& time, const float& fElapsedTime)
{
	auto context = D3D->GetDeviceContext();

	// ビュー行列の設定
	XMMATRIX view = GAME->GetCameraView();
	m_CollisionShader->SetView(view);

	XMVECTOR location = m_Collision->GetLocation();
	XMVECTOR rotation = m_Collision->GetActor()->GetRotation();

	XMMATRIX world = XMMatrixRotationRollPitchYawFromVector(rotation) * XMMatrixTranslationFromVector(location);
	m_CollisionShader->SetWorld(world);

	// プリミティブトポロジーの設定
	context->IASetPrimitiveTopology(m_PrimitiveTopology);

	// 頂点バッファの設定
	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);

	// インデックスバッファの設定
	context->IASetIndexBuffer(m_IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0);

	// シェーダの設定
	m_CollisionShader->SetRenderShader();

	context->DrawIndexed(m_IndexNum, 0, 0);

	Actor::FrameRender(time, fElapsedTime);
}

void CheckCollision::OnResizedSwapChain()
{
	// 射影行列の設定
	auto screen = D3D->GetScreenViewport();
	auto aspect = screen.Width / screen.Height;
	auto projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 0.01f, 1000.0f);
	m_CollisionShader->SetProjection(projection);

	Actor::OnResizedSwapChain();
}

HRESULT CheckCollision::CreateCapsuleVertex(float radius, float height)
{
	HRESULT hr = S_OK;

	m_IndexNum = (UINT)indexNum;
	radius = radius <= 0.0f ? 0.1f : radius;
	height = height <= 0.0f ? 0.0f : height;
	m_PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

	// 頂点データの作成
	SimpleVertex	vertices[vertexNum] = {};
	float			halfHeight = height * 0.5f;
	constexpr float vDev = 1.0f / vMax;
	constexpr float uDev = 1.0f / uMax;
	constexpr int   vHalf = static_cast<int>(vMax * 0.5f);

	int	index = 0;
	for (int v = 0; v <= vMax; v++) {
		// 上半球
		if (v <= vHalf) {
			for (int u = 0; u < uMax; u++) {
				double theta = XM_PI * ((float)v * vDev);
				double phi   = XM_2PI * ((float)u * uDev);
				float x = static_cast<float>(sin(theta) * cos(phi) * radius);
				float y = static_cast<float>(cos(theta) * radius + halfHeight);
				float z = static_cast<float>(sin(theta) * sin(phi) * radius);
				vertices[index++].Pos = XMFLOAT3(x, y, z);
			}
		}
		// 下半球
		if (v >= vHalf) {
			for (int u = 0; u < uMax; u++) {
				double theta = XM_PI * ((float)v * vDev);
				double phi = XM_2PI * ((float)u * uDev);
				float x = static_cast<float>(sin(theta) * cos(phi) * radius);
				float y = static_cast<float>(cos(theta) * radius - halfHeight);
				float z = static_cast<float>(sin(theta) * sin(phi) * radius);
				vertices[index++].Pos = XMFLOAT3(x, y, z);
			}
		}
	}

	// 頂点バッファの作成
	D3D11_BUFFER_DESC verDesc = {};
	verDesc.Usage = D3D11_USAGE_DEFAULT;
	verDesc.ByteWidth = sizeof(SimpleVertex) * vertexNum;
	verDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = &vertices;
	hr = D3D->GetDevice()->CreateBuffer(&verDesc, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	// インデックス情報の作成
	WORD indices[indexNum] = {};
	index = 0;
	for (int v = 0; v < vMax; v++) {
		for (int u = 0; u <= uMax; u++) {
			if (u == uMax) {
				indices[index++] = v * uMax;
				indices[index++] = (v + 1) * uMax;
			}
			else {
				indices[index++] = (v * uMax) + u;
				indices[index] = indices[index - 1] + uMax;
				index++;
			}
		}
	}

	// インデックスバッファの作成
	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.ByteWidth = sizeof(WORD) * indexNum;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	InitData.pSysMem = &indices;
	hr = D3D->GetDevice()->CreateBuffer(&indexDesc, &InitData, m_IndexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT CheckCollision::CreateBoxVertex(XMVECTOR extent)
{
	HRESULT hr = S_OK;
	
	m_PrimitiveTopology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	SimpleVertex vertices[] = {
		{{-extent.m128_f32[0],  extent.m128_f32[1],  extent.m128_f32[2]}},
		{{ extent.m128_f32[0],  extent.m128_f32[1],  extent.m128_f32[2]}},
		{{-extent.m128_f32[0], -extent.m128_f32[1],  extent.m128_f32[2]}},
		{{ extent.m128_f32[0], -extent.m128_f32[1],  extent.m128_f32[2]}},
		{{-extent.m128_f32[0],  extent.m128_f32[1], -extent.m128_f32[2]}},
		{{ extent.m128_f32[0],  extent.m128_f32[1], -extent.m128_f32[2]}},
		{{-extent.m128_f32[0], -extent.m128_f32[1], -extent.m128_f32[2]}},
		{{ extent.m128_f32[0], -extent.m128_f32[1], -extent.m128_f32[2]}}
	};

	D3D11_BUFFER_DESC verDesc = {};
	verDesc.Usage = D3D11_USAGE_DEFAULT;
	verDesc.ByteWidth = sizeof(SimpleVertex) * ARRAYSIZE(vertices);
	verDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	verDesc.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = &vertices;
	hr = D3D->GetDevice()->CreateBuffer(&verDesc, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	WORD indices[] =
	{
		0, 1, 3,
		3, 2, 0,
		
		5, 4, 6,
		6, 7, 5,
		
		4, 0, 2,
		2, 6, 4,
		
		1, 5, 7,
		7, 3, 1,
		
		4, 5, 1,
		1, 0, 4,
		
		2, 3, 7,
		7, 6, 2
	};

	m_IndexNum = ARRAYSIZE(indices);
	D3D11_BUFFER_DESC indexDesc = {};
	indexDesc.Usage = D3D11_USAGE_DEFAULT;
	indexDesc.ByteWidth = sizeof(WORD) * m_IndexNum;
	indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexDesc.CPUAccessFlags = 0;
	InitData.pSysMem = &indices;
	hr = D3D->GetDevice()->CreateBuffer(&indexDesc, &InitData, m_IndexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	return S_OK;
}