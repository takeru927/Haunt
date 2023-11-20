#include "Effect.h"
#include "GameLibrary/DirectDevice.h"
#include "GameLibrary/WICTextureLoader11.h"
#include <ctime>
#include <cstdlib>

#include "GameMain.h"

namespace
{
	struct EffectVertex {
		XMFLOAT3 location;
		XMFLOAT4 color;
		XMFLOAT2 textureUV;
	};
}

Effect::Effect(LPCWSTR fileName, const int num)
	: Actor(L"Effect")
	, m_TextureRV(nullptr),m_VertexBuffer(nullptr)
{
	wcscpy_s(m_FileName, fileName);
	m_Status = EActorStatus::NOUSE;

	// 指定された数だけエフェクトを作成
	m_NumNode = num < 1 ? 1 : num;
	for (int i = 0; i < m_NumNode; i++) {
		auto node = std::make_unique<EffectNode>();
		node->m_On = false;
		m_Node.push_back(std::move(node));
	}
	Initialize();

	// シェーダの作成
	m_Shader = std::make_unique<EffectShader>();
}

void Effect::Initialize(const float scale, const int sizeX, const int sizeY)
{
	m_Scale = scale;
	m_SizeX = sizeX;
	m_SizeY = sizeY;
	m_Size = sizeX * sizeY;
}

HRESULT Effect::CreateObject()
{
	HRESULT hr = S_OK;

	// シェーダの作成
	hr = m_Shader->CreateShader();
	if (FAILED(hr))
		return hr;

	// テクスチャの読み込み
	hr = CreateWICTextureFromFileEx(
		D3D->GetDevice(), 
		D3D->GetDeviceContext(), 
		m_FileName,
		0, 
		D3D11_USAGE_DEFAULT, 
		D3D11_BIND_SHADER_RESOURCE, 
		0, 0,
		WIC_LOADER_FORCE_SRGB,
		nullptr,
		m_TextureRV.ReleaseAndGetAddressOf()
	);
	if (FAILED(hr))
		return hr;

	// 頂点バッファの作成
	hr = CreateVertex();
	if (FAILED(hr))
		return hr;

	hr = Actor::CreateObject();
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT Effect::PlayEffect(CXMVECTOR location, const float spanTime, const float startTime)
{
	// 既に再生中なら何もしない
	if (m_Status == EActorStatus::ACTIVE)
		return E_FAIL;

	// 再生開始
	m_Status = EActorStatus::ACTIVE;
	m_Node[0]->m_On = true;
	m_Node[0]->m_Location = location;
	m_Node[0]->m_Span = spanTime;
	m_Node[0]->m_Time = 0.0f;
	m_Node[0]->m_Texture = 0;

	// エフェクトを複数出す場合の、2つ目以降の設定
	XMVECTOR locate = {};
	srand((unsigned)time(NULL));
	for (int i = 1; i < m_NumNode; i++) {
		// 乱数で少し位置を変える
		locate.m128_f32[0] = location.m128_f32[0] + m_Scale * ((float)rand() / RAND_MAX - 0.5f);
		locate.m128_f32[1] = location.m128_f32[1] + m_Scale * ((float)rand() / RAND_MAX - 0.5f);
		locate.m128_f32[2] = location.m128_f32[2] + m_Scale * ((float)rand() / RAND_MAX - 0.5f);

		m_Node[i]->m_On = true;
		m_Node[i]->m_Location = locate;
		m_Node[i]->m_Span = spanTime * (0.5f + (float)rand() / RAND_MAX);;
		m_Node[i]->m_Time = -startTime * i;
		m_Node[i]->m_Texture = 0;
	}
	
	return S_OK;
}

void Effect::FrameMove(const double& time, const float& fElapsedTime)
{
	bool bLive = false;
	for (int i = 0; i < m_NumNode; i++) {
		// 再生されてないものは飛ばす
		if (m_Node[i]->m_On == false)
			continue;

		// 時間経過で次のコマに変える
		m_Node[i]->m_Time += fElapsedTime;
		if (m_Node[i]->m_Time > m_Node[i]->m_Span) {
			m_Node[i]->m_Time -= m_Node[i]->m_Span;
			// 最後のコマまで行ったら、再生終了フラグを立てる
			if (++m_Node[i]->m_Texture >= m_Size)
				m_Node[i]->m_On = false;
			// まだ再生中
			else
				bLive = true;
		}
		// まだ再生中
		else
			bLive = true;
	}

	// 再生終了
	if (bLive == false)
		m_Status = EActorStatus::NOUSE;

	Actor::FrameMove(time, fElapsedTime);
}

void Effect::FrameRender(const double& time, const float& fElapsedTime)
{
	// 再生中でなければ、何もしない
	if (m_Status != EActorStatus::ACTIVE)
		return;

	// ビュー行列の設定
	XMMATRIX view = GAME->GetCameraView();
	m_Shader->SetView(view);

	// 正面をカメラ方向に向ける
	view.r[3].m128_f32[0] = view.r[3].m128_f32[1] = view.r[3].m128_f32[2] = 0.0f;
	XMMATRIX rotate = XMMatrixInverse(nullptr, view);

	// 頂点バッファの設定
	auto context = D3D->GetDeviceContext();
	UINT stride = sizeof(EffectVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// テクスチャの設定
	context->PSSetShaderResources(0, 1, m_TextureRV.GetAddressOf());

	for (int i = 0; i < m_NumNode; i++) {
		// 再生されてないものは飛ばす
		if (m_Node[i]->m_On == false ||
			m_Node[i]->m_Time < 0.0f)
			continue;

		// ワールド行列の設定
		XMMATRIX world = rotate *
			XMMatrixTranslation(m_Node[i]->m_Location.m128_f32[0],
								m_Node[i]->m_Location.m128_f32[1],
								m_Node[i]->m_Location.m128_f32[2]);
		m_Shader->SetWorld(world);
		// シェーダの設定
		m_Shader->SetRenderShader();

		// 爆発の描画
		offset = m_Node[i]->m_Texture * 4;
		context->Draw(4, offset);
	}

	Actor::FrameRender(time, fElapsedTime);
}

void Effect::OnResizedSwapChain()
{
	// 射影行列の設定
	auto screen = D3D->GetScreenViewport();
	auto aspect = screen.Width / screen.Height;
	auto projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 0.01f, 1000.0f);
	m_Shader->SetProjection(projection);

	Actor::OnResizedSwapChain();
}

HRESULT Effect::CreateVertex()
{
	HRESULT hr = S_OK;

	// 頂点情報の設定
	int   verNum = m_Size * 4;
	auto  vertex = new EffectVertex[verNum];
	float frameSizeX = 1.0f / (float)m_SizeX;
	float frameSizeY = 1.0f / (float)m_SizeY;

	for (int i = 0, frame = 0; i < verNum; i += 4, frame++) {
		// 頂点座標
		vertex[i + 0].location = { -m_Scale,  m_Scale, 0.0f };
		vertex[i + 1].location = {  m_Scale,  m_Scale, 0.0f };
		vertex[i + 2].location = { -m_Scale, -m_Scale, 0.0f };
		vertex[i + 3].location = {  m_Scale, -m_Scale, 0.0f };

		// UV座標
		float texU = frameSizeX * float(frame % m_SizeX);
		float texV = frameSizeY * float(frame / m_SizeX);
		vertex[i + 0].textureUV = { texU, texV };
		vertex[i + 1].textureUV = { texU + frameSizeX, texV };
		vertex[i + 2].textureUV = { texU, texV + frameSizeY };
		vertex[i + 3].textureUV = { texU + frameSizeX, texV + frameSizeY };

		// カラー
		float alpha = 1.0f - float(frame) / float(m_Size);
		vertex[i + 0].color = { 1.0f, 1.0f, 1.0f, alpha };
		vertex[i + 1].color = { 1.0f, 1.0f, 1.0f, alpha };
		vertex[i + 2].color = { 1.0f, 1.0f, 1.0f, alpha };
		vertex[i + 3].color = { 1.0f, 1.0f, 1.0f, alpha };
	}

	// 頂点バッファの作成
	D3D11_BUFFER_DESC bd = {};
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(EffectVertex) * 4 * m_Size;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData = {};
	InitData.pSysMem = vertex;
	hr = D3D->GetDevice()->CreateBuffer(&bd, &InitData, m_VertexBuffer.ReleaseAndGetAddressOf());
	if (FAILED(hr))
		return hr;

	delete[] vertex;

	return S_OK;
}