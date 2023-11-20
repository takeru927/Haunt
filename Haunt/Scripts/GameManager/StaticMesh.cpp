#include "StaticMesh.h"
#include "GameLibrary/DirectDevice.h"

StaticMesh::StaticMesh() : m_MaterialNum(0)
{

}

HRESULT StaticMesh::CreateModel(LPCWSTR fileName)
{
	HRESULT hr = S_OK;

	wchar_t fileFormat[4] = {};
	size_t length = wcslen(fileName);
	for (int i = 0; i < 3; i++)
		fileFormat[i] = fileName[length - (3 - i)];

	if (wcscmp(fileFormat, L"obj") == 0)
		hr = MeshImporter::LoadObjMesh(fileName, m_Meshs, m_Materials, m_MaterialNum);

	else if (wcscmp(fileFormat, L"fbx") == 0)
		hr = MeshImporter::LoadFbxMesh(fileName, m_Meshs, m_Materials, m_MaterialNum);

	else
		return E_FAIL;

	if (FAILED(hr))
		return hr;

	return S_OK;
}

void StaticMesh::RenderModel(std::function<void(const Material*)> shaderSetting)
{
	auto context = D3D->GetDeviceContext();

	for (int i = 0; i < m_MaterialNum; i++)
	{
		std::string matName(m_Meshs[i].MaterialName, strlen(m_Meshs[i].MaterialName));

		// シェーダの設定
		if (shaderSetting)
			shaderSetting(&m_Materials[matName]);

		// プリミティブトポロジーの設定
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 頂点バッファの設定
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		context->IASetVertexBuffers(0, 1, m_Meshs[i].VertexBuffer.GetAddressOf(), &stride, &offset);

		// インデックスバッファの設定
		context->IASetIndexBuffer(m_Meshs[i].IndexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

		// テクスチャの設定
		if (m_Materials[matName].Texture != nullptr)
			context->PSSetShaderResources(0, 1, m_Materials[matName].Texture.GetAddressOf());
		else {
			ID3D11ShaderResourceView* srv[] = { nullptr };
			context->PSSetShaderResources(0, 1, srv);
		}

		// 描画
		context->DrawIndexed((UINT)m_Meshs[i].Indices.size(), 0, 0);
	}
}

bool StaticMesh::Intersect(const XMVECTOR rayPos, const XMVECTOR rayDir, const XMMATRIX* const world, float* const dist)
{
	// 計算用の変数宣言
	XMVECTOR vert1, vert2, vert3;
	XMVECTOR faceNormal, planePoint;
	XMVECTOR triCross1, triCross2;
	float x, y, z, plane;
	float ep1, ep2, distance;
	bool hit = false;
	*dist = 10000.0f;	// 比較用に値を大きくしておく

	for (int i = 0; i < m_MaterialNum; i++) {
		for (DWORD j = 0; j < m_Meshs[i].Vertices.size(); j += 3) {
			// 三角形の頂点の取得
			vert1 = XMLoadFloat3(&m_Meshs[i].Vertices[j].Pos);
			vert2 = XMLoadFloat3(&m_Meshs[i].Vertices[j + 1].Pos);
			vert3 = XMLoadFloat3(&m_Meshs[i].Vertices[j + 2].Pos);

			// ワールド座標に変換
			vert1 = XMVector3TransformCoord(vert1, *world);
			vert2 = XMVector3TransformCoord(vert2, *world);
			vert3 = XMVector3TransformCoord(vert3, *world);

			// 三角形の法線の算出
			faceNormal = XMVector3Cross(vert2 - vert1, vert3 - vert1);
			faceNormal = XMVector3Normalize(faceNormal);

			// レイが裏側から当たっている場合、無視する
			if (XMVectorGetX(XMVector3Dot(faceNormal, rayDir)) > 0)
			{
				continue;
			}

			// 面の法線の各要素の取得
			x = XMVectorGetX(faceNormal);
			y = XMVectorGetY(faceNormal);
			z = XMVectorGetZ(faceNormal);

			// 三角形がある平面の式を算出
			plane = -(x * XMVectorGetX(vert1) + y * XMVectorGetY(vert1) + z * XMVectorGetZ(vert1));

			// レイの衝突距離を算出
			ep1 = (XMVectorGetX(rayPos) * x) + (XMVectorGetY(rayPos) * y) + (XMVectorGetZ(rayPos) * z);
			ep2 = (XMVectorGetX(rayDir) * x) + (XMVectorGetY(rayDir) * y) + (XMVectorGetZ(rayDir) * z);
			distance = 0.0f;
			if (ep2 != 0.0f)	// 0除算対策
				distance = -(ep1 + plane) / ep2;

			// 三角形とヒットしているか？
			if (distance > 0.0f) {
				// 平面のどこにヒットしているか算出
				planePoint = rayPos + rayDir * distance;

				// ヒット位置が三角形の中にあるか？
				// 点が内側にあるとき、各頂点の外積ベクトルは同じ向きになる
				triCross1 = XMVector3Cross(vert3 - vert2, planePoint - vert3);
				triCross2 = XMVector3Cross(vert1 - vert3, planePoint - vert1);

				if (XMVectorGetX(XMVector3Dot(triCross1, triCross2)) >= 0)
				{
					triCross2 = XMVector3Cross(vert2 - vert1, planePoint - vert2);
					if (XMVectorGetX(XMVector3Dot(triCross1, triCross2)) >= 0)
					{
						if (*dist > distance) {
							*dist = distance;
						}

						hit = true;
					}
				}
			}
		}
	}

	// ヒットしていなければ、値を戻す
	if (hit == false) *dist = 0.0f;

	return hit;
}