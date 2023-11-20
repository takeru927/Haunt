#include "SkeltalMesh.h"
#include "GameLibrary/DirectDevice.h"

SkeltalMesh::SkeltalMesh() : m_MaterialNum(0)
{

}

HRESULT SkeltalMesh::CreateModel(LPCWSTR fileName)
{
	HRESULT hr = S_OK;

	wchar_t fileFormat[4] = {};
	size_t length = wcslen(fileName);
	for (int i = 0; i < 3; i++)
		fileFormat[i] = fileName[length - (3 - i)];

	if (wcscmp(fileFormat, L"fbx") == 0)
		hr = MeshImporter::LoadFbxAnimMesh(fileName, m_Meshs, m_Bones, m_Materials, m_MaterialNum);

	else
		return E_FAIL;

	if (FAILED(hr))
		return hr;

	return S_OK;
}

void SkeltalMesh::RenderModel(std::function<void(const Material*)> shaderSetting)
{
	auto context = D3D->GetDeviceContext();

	for (int i = 0; i < m_MaterialNum; i++)
	{
		std::string matName(m_Meshs[i].MaterialName, strlen(m_Meshs[i].MaterialName));

		// �V�F�[�_�̐ݒ�
		if (shaderSetting)
			shaderSetting(&m_Materials[matName]);

		// �v���~�e�B�u�g�|���W�[�̐ݒ�
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// ���_�o�b�t�@�̐ݒ�
		UINT stride = sizeof(VertexAnim);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, m_Meshs[i].VertexBuffer.GetAddressOf(), &stride, &offset);

		// �C���f�b�N�X�o�b�t�@�̐ݒ�
		context->IASetIndexBuffer(m_Meshs[i].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// �e�N�X�`���̐ݒ�
		if (m_Materials[matName].Texture != nullptr)
			context->PSSetShaderResources(0, 1, m_Materials[matName].Texture.GetAddressOf());
		else {
			ID3D11ShaderResourceView* srv[] = { nullptr };
			context->PSSetShaderResources(0, 1, srv);
		}

		// �`��
		context->DrawIndexed((UINT)m_Meshs[i].Indices.size(), 0, 0);
	}
}

std::unordered_map<std::string, UINT>& SkeltalMesh::GetBones()
{
	return m_Bones;
}