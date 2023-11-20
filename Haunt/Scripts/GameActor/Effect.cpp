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

	// �w�肳�ꂽ�������G�t�F�N�g���쐬
	m_NumNode = num < 1 ? 1 : num;
	for (int i = 0; i < m_NumNode; i++) {
		auto node = std::make_unique<EffectNode>();
		node->m_On = false;
		m_Node.push_back(std::move(node));
	}
	Initialize();

	// �V�F�[�_�̍쐬
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

	// �V�F�[�_�̍쐬
	hr = m_Shader->CreateShader();
	if (FAILED(hr))
		return hr;

	// �e�N�X�`���̓ǂݍ���
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

	// ���_�o�b�t�@�̍쐬
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
	// ���ɍĐ����Ȃ牽�����Ȃ�
	if (m_Status == EActorStatus::ACTIVE)
		return E_FAIL;

	// �Đ��J�n
	m_Status = EActorStatus::ACTIVE;
	m_Node[0]->m_On = true;
	m_Node[0]->m_Location = location;
	m_Node[0]->m_Span = spanTime;
	m_Node[0]->m_Time = 0.0f;
	m_Node[0]->m_Texture = 0;

	// �G�t�F�N�g�𕡐��o���ꍇ�́A2�ڈȍ~�̐ݒ�
	XMVECTOR locate = {};
	srand((unsigned)time(NULL));
	for (int i = 1; i < m_NumNode; i++) {
		// �����ŏ����ʒu��ς���
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
		// �Đ�����ĂȂ����͔̂�΂�
		if (m_Node[i]->m_On == false)
			continue;

		// ���Ԍo�߂Ŏ��̃R�}�ɕς���
		m_Node[i]->m_Time += fElapsedTime;
		if (m_Node[i]->m_Time > m_Node[i]->m_Span) {
			m_Node[i]->m_Time -= m_Node[i]->m_Span;
			// �Ō�̃R�}�܂ōs������A�Đ��I���t���O�𗧂Ă�
			if (++m_Node[i]->m_Texture >= m_Size)
				m_Node[i]->m_On = false;
			// �܂��Đ���
			else
				bLive = true;
		}
		// �܂��Đ���
		else
			bLive = true;
	}

	// �Đ��I��
	if (bLive == false)
		m_Status = EActorStatus::NOUSE;

	Actor::FrameMove(time, fElapsedTime);
}

void Effect::FrameRender(const double& time, const float& fElapsedTime)
{
	// �Đ����łȂ���΁A�������Ȃ�
	if (m_Status != EActorStatus::ACTIVE)
		return;

	// �r���[�s��̐ݒ�
	XMMATRIX view = GAME->GetCameraView();
	m_Shader->SetView(view);

	// ���ʂ��J���������Ɍ�����
	view.r[3].m128_f32[0] = view.r[3].m128_f32[1] = view.r[3].m128_f32[2] = 0.0f;
	XMMATRIX rotate = XMMatrixInverse(nullptr, view);

	// ���_�o�b�t�@�̐ݒ�
	auto context = D3D->GetDeviceContext();
	UINT stride = sizeof(EffectVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, m_VertexBuffer.GetAddressOf(), &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	// �e�N�X�`���̐ݒ�
	context->PSSetShaderResources(0, 1, m_TextureRV.GetAddressOf());

	for (int i = 0; i < m_NumNode; i++) {
		// �Đ�����ĂȂ����͔̂�΂�
		if (m_Node[i]->m_On == false ||
			m_Node[i]->m_Time < 0.0f)
			continue;

		// ���[���h�s��̐ݒ�
		XMMATRIX world = rotate *
			XMMatrixTranslation(m_Node[i]->m_Location.m128_f32[0],
								m_Node[i]->m_Location.m128_f32[1],
								m_Node[i]->m_Location.m128_f32[2]);
		m_Shader->SetWorld(world);
		// �V�F�[�_�̐ݒ�
		m_Shader->SetRenderShader();

		// �����̕`��
		offset = m_Node[i]->m_Texture * 4;
		context->Draw(4, offset);
	}

	Actor::FrameRender(time, fElapsedTime);
}

void Effect::OnResizedSwapChain()
{
	// �ˉe�s��̐ݒ�
	auto screen = D3D->GetScreenViewport();
	auto aspect = screen.Width / screen.Height;
	auto projection = XMMatrixPerspectiveFovLH(XM_PIDIV2, aspect, 0.01f, 1000.0f);
	m_Shader->SetProjection(projection);

	Actor::OnResizedSwapChain();
}

HRESULT Effect::CreateVertex()
{
	HRESULT hr = S_OK;

	// ���_���̐ݒ�
	int   verNum = m_Size * 4;
	auto  vertex = new EffectVertex[verNum];
	float frameSizeX = 1.0f / (float)m_SizeX;
	float frameSizeY = 1.0f / (float)m_SizeY;

	for (int i = 0, frame = 0; i < verNum; i += 4, frame++) {
		// ���_���W
		vertex[i + 0].location = { -m_Scale,  m_Scale, 0.0f };
		vertex[i + 1].location = {  m_Scale,  m_Scale, 0.0f };
		vertex[i + 2].location = { -m_Scale, -m_Scale, 0.0f };
		vertex[i + 3].location = {  m_Scale, -m_Scale, 0.0f };

		// UV���W
		float texU = frameSizeX * float(frame % m_SizeX);
		float texV = frameSizeY * float(frame / m_SizeX);
		vertex[i + 0].textureUV = { texU, texV };
		vertex[i + 1].textureUV = { texU + frameSizeX, texV };
		vertex[i + 2].textureUV = { texU, texV + frameSizeY };
		vertex[i + 3].textureUV = { texU + frameSizeX, texV + frameSizeY };

		// �J���[
		float alpha = 1.0f - float(frame) / float(m_Size);
		vertex[i + 0].color = { 1.0f, 1.0f, 1.0f, alpha };
		vertex[i + 1].color = { 1.0f, 1.0f, 1.0f, alpha };
		vertex[i + 2].color = { 1.0f, 1.0f, 1.0f, alpha };
		vertex[i + 3].color = { 1.0f, 1.0f, 1.0f, alpha };
	}

	// ���_�o�b�t�@�̍쐬
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