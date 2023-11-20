#include "MeshImporter.h"
#include "fbxsdk.h"
#include "GameLibrary/DirectDevice.h"

namespace
{
	// 頂点バッファ、インデックスバッファの作成
	HRESULT CreateBuffers(std::vector<MeshNode>& meshs, int& materialNum)
	{
		HRESULT hr = S_OK;
		auto device = D3D->GetDevice();

		for (int i = 0; i < materialNum; i++)
		{
			// 頂点バッファの作成
			D3D11_BUFFER_DESC verDesc = {};
			verDesc.Usage = D3D11_USAGE_DEFAULT;
			verDesc.ByteWidth = sizeof(Vertex) * (UINT)meshs[i].Vertices.size();
			verDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			verDesc.CPUAccessFlags = 0;

			D3D11_SUBRESOURCE_DATA InitData = {};
			InitData.pSysMem = meshs[i].Vertices.data();
			hr = device->CreateBuffer(&verDesc, &InitData, meshs[i].VertexBuffer.ReleaseAndGetAddressOf());
			if (FAILED(hr))
				return hr;

			// インデックスバッファの作成
			D3D11_BUFFER_DESC indexDesc = {};
			indexDesc.Usage = D3D11_USAGE_DEFAULT;
			indexDesc.ByteWidth = sizeof(DWORD) * (UINT)meshs[i].Indices.size();
			indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexDesc.CPUAccessFlags = 0;
			InitData.pSysMem = meshs[i].Indices.data();
			hr = device->CreateBuffer(&indexDesc, &InitData, meshs[i].IndexBuffer.ReleaseAndGetAddressOf());
			if (FAILED(hr))
				return hr;
		}

		return S_OK;
	}
}

//--------------------------------------------------------------------------------------
// objファイルのロード
//--------------------------------------------------------------------------------------

HRESULT MeshImporter::LoadObjMesh(LPCWSTR fileName,
								  std::vector<MeshNode>& meshs,
								  std::unordered_map<std::string, Material>& materials,
								  int& materialNum)
{
	HRESULT hr = S_OK;

	WCHAR filePath[256] = { };
	WCHAR mtlFileName[256] = { };
	WCHAR texFileName[256] = { };

	const int dataLength = 512;
	char data[dataLength] = { 0 };
	char* headPoint = nullptr;
	const int matNameLength = 128;
	CHAR currentMatName[matNameLength] = {};
	int matIndex = -1;

	std::vector<XMFLOAT3> vertices;
	std::vector<XMFLOAT3> normals;
	std::vector<XMFLOAT2> textures;

	// ファイルパスの取得
	for (int i = lstrlenW(fileName) - 1; i > 0; --i) {
		if (fileName[i] == '/') {
			wcsncpy_s(filePath, fileName, static_cast<size_t>(i + 1));
			break;
		}
	}

	// objファイルを開いて内容を読み込む
	FILE* fp = nullptr;
	_wfopen_s(&fp, fileName, L"r");
	if (fp == nullptr)
		return E_FAIL;

	// メッシュの読み込み
	while (fgets(data, dataLength, fp) != nullptr)
	{
		// コメントは無視する
		if (data[0] == '#')
		{
			continue;
		}

		// データの先頭アドレスを取得する
		headPoint = strchr(data, ' ');
		if (headPoint == nullptr) continue;
		headPoint += 1;

		if (data[0] == 'v')
		{
			// 頂点座標
			if (data[1] == ' ') {
				float x, y, z;
				sscanf_s(headPoint, "%f %f %f", &x, &y, &z);

				XMFLOAT3 vertex = { x,y,z };
				vertices.emplace_back(vertex);
			}

			// UV座標
			else if (data[1] == 't') {
				float x, y;
				sscanf_s(headPoint, "%f %f", &x, &y);

				y = 1.0f - y;	// Y軸を反転させる

				XMFLOAT2 texture = { x,y };
				textures.emplace_back(texture);
			}

			// 法線ベクトル
			else if (data[1] == 'n')
			{
				float x, y, z;
				sscanf_s(headPoint, "%f %f %f", &x, &y, &z);

				XMFLOAT3 normal = { x,y,z };
				normals.emplace_back(normal);
			}
		}

		// 面情報
		else if (data[0] == 'f')
		{
			int face[3] = { 0 };

			// 三角形
			for (int i = 1;; i++) {
				// 1頂点のデータを取得する
				for (int j = 0; j < 3; j++) {
					sscanf_s(headPoint, "%d", &face[j]);

					if (j >= 2) break;	// 最後のデータなら、移動させずに終了する

					// 次の座標データに移動させる
					headPoint = strchr(headPoint, '/');
					if (headPoint == nullptr) continue;
					headPoint += 1;
				}

				Vertex vertex;
				if (textures.size() >= face[1] - 1) {
					vertex = {
						vertices[face[0] - 1],
						normals[face[2] - 1],
						textures[face[1] - 1]
					};
				}
				else {
					vertex = {
						vertices[face[0] - 1],
						normals[face[2] - 1],
						{0.0f, 0.0f}
					};
				}

				// 4頂点以降なら、インデックスを追加する
				if (i >= 4) {
					meshs[matIndex].Indices.emplace_back((DWORD)meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 3]);
					meshs[matIndex].Indices.emplace_back((DWORD)meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 2]);
				}

				// 頂点を追加し、そのインデックスを格納
				meshs[matIndex].Vertices.emplace_back(vertex);
				meshs[matIndex].Indices.emplace_back((DWORD)meshs[matIndex].Vertices.size() - 1);

				// 次の頂点データに移動させる
				headPoint = strchr(headPoint, ' ');
				if (headPoint == nullptr) break;	// データがなければ終了
			}
		}

		// 使用するマテリアル名
		else if (strstr(data, "usemtl") != nullptr)
		{
			sscanf_s(headPoint, "%s", currentMatName, matNameLength);

			MeshNode mesh = {};
			strcpy_s(mesh.MaterialName, currentMatName);
			meshs.emplace_back(mesh);

			matIndex++;
		}

		// mtlファイル情報
		else if (strstr(data, "mtllib") != nullptr)
		{
			// mtlファイル名のコピー
			WCHAR mtl[matNameLength] = {};
			sscanf_s(headPoint, "%ls", mtl, matNameLength);
			wcscpy_s(mtlFileName, filePath);
			wcscat_s(mtlFileName, mtl);
		}
	}

	// objファイルを閉じる
	fclose(fp);
	fp = nullptr;

	// データ取得用の一時変数
	CHAR matName[matNameLength] = { };

	// mtlファイルを開いて、内容を読み込む
	_wfopen_s(&fp, mtlFileName, L"r");
	if (fp == nullptr)
		return E_FAIL;

	// ポインタを先頭に戻す
	fseek(fp, SEEK_SET, 0);

	// マテリアルの読み込み
	while (fgets(data, dataLength, fp) != nullptr)
	{
		// コメントは無視する
		if (data[0] == '#')
		{
			continue;
		}

		// データの先頭アドレスを取得する
		char* headPoint = strchr(data, ' ') + 1;
		if (headPoint == nullptr)
		{
			continue;
		}

		// マテリアル名
		if (strstr(data, "newmtl") != nullptr)
		{
			materialNum++;
			sscanf_s(headPoint, "%s", matName, matNameLength);
		}

		else if (data[0] == 'K')
		{
			// アンビエント
			if (data[1] == 'a')
			{
				float r, g, b;
				sscanf_s(headPoint, "%f %f %f", &r, &g, &b);

				XMFLOAT4 ambient = { r,g,b, 1.0f };
				materials[matName].Ambient = ambient;
			}

			// ディフューズ
			else if (data[1] == 'd')
			{
				float r, g, b;
				sscanf_s(headPoint, "%f %f %f", &r, &g, &b);

				XMFLOAT4 diffuse = { r,g,b, 1.0f };
				materials[matName].Diffuse = diffuse;
			}

			// エミッシブ
			else if (data[1] == 'e')
			{
				float r, g, b;
				sscanf_s(headPoint, "%f %f %f", &r, &g, &b);

				XMFLOAT4 emissive = { r,g,b, 1.0f };
				materials[matName].Emissive = emissive;
			}
		}

		// テクスチャ
		else if (strstr(data, "map_Kd") != nullptr)
		{
			WCHAR texName[matNameLength] = {};
			sscanf_s(headPoint, "%ls", texName, matNameLength);

			// テクスチャファイルパスの取得
			for (int i = lstrlenW(filePath) - 2; i > 0; --i) {
				if (filePath[i] == '/') {
					wcsncpy_s(texFileName, filePath, static_cast<size_t>(i + 1));
					break;
				}
			}
			wcscat_s(texFileName, L"Textures/");
			wcscat_s(texFileName, texName);

			hr = CreateWICTextureFromFileEx(
				D3D->GetDevice(),
				D3D->GetDeviceContext(),
				texFileName,
				0,
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE,
				0, 0,
				WIC_LOADER_FORCE_SRGB,
				nullptr,
				materials[matName].Texture.ReleaseAndGetAddressOf()
			);
			if (FAILED(hr))
				return hr;
		}
	}

	// mtlファイルを閉じる
	fclose(fp);

	CreateBuffers(meshs, materialNum);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// fbxファイルのロード
//--------------------------------------------------------------------------------------

namespace
{
	struct BoneWeight
	{
		XMFLOAT4 BoneIndex;
		XMFLOAT4 Weight;
	};

	//--------------------------------------------------------------------------------------
	// fbxマテリアルの読み込み

	HRESULT LoadFbxMaterial(FbxSurfaceMaterial* surfaceMaterial,
							LPCWSTR fileName,
							CHAR(&outMatName)[256],
							std::unordered_map<std::string, Material>& materials)
	{
		FbxDouble3 color;

		std::string matName(surfaceMaterial->GetName());
		strcpy_s(outMatName, surfaceMaterial->GetName());

		if (surfaceMaterial->GetClassId().Is(FbxSurfacePhong::ClassId)) {
			color = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbient).Get<FbxDouble3>();
			materials[matName].Ambient = { (float)color[0], (float)color[1], (float)color[2], 1.0f };

			color = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse).Get<FbxDouble3>();
			materials[matName].Diffuse = { (float)color[0], (float)color[1], (float)color[2], 1.0f };

			color = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sEmissive).Get<FbxDouble3>();
			materials[matName].Emissive = { (float)color[0], (float)color[1], (float)color[2], 1.0f };
		}

		else if (surfaceMaterial->GetClassId().Is(FbxSurfaceLambert::ClassId)) {
			color = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sAmbient).Get<FbxDouble3>();
			materials[matName].Ambient = { (float)color[0], (float)color[1], (float)color[2], 1.0f };

			color = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse).Get<FbxDouble3>();
			materials[matName].Diffuse = { (float)color[0], (float)color[1], (float)color[2], 1.0f };

			color = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sEmissive).Get<FbxDouble3>();
			materials[matName].Emissive = { (float)color[0], (float)color[1], (float)color[2], 1.0f };
		}

		// テクスチャの取得
		auto prop = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		int texCount = prop.GetSrcObjectCount<FbxFileTexture>();

		if (texCount > 0) {
			auto texture = prop.GetSrcObject<FbxFileTexture>(0);

			// テクスチャの相対パスを取得
			auto mbTexFilePath = texture->GetRelativeFileName();
			wchar_t wTexFilePath[256] = {};
			for (int i = 0; i < strlen(mbTexFilePath); i++) {
				if (mbTexFilePath[i] == '\\')
					wTexFilePath[i] = '/';
				else
					wTexFilePath[i] = mbTexFilePath[i];
			}

			// モデルがあるファイルまでのパスを取得
			wchar_t texFileName[256] = {};
			for (size_t i = wcslen(fileName) - 1; i > 0; --i) {
				if (fileName[i] == '/') {
					wcsncpy_s(texFileName, _countof(texFileName), fileName, i + 1);
					break;
				}
			}
			// テクスチャまでのファイルパスを作成
			wcscat_s(texFileName, wTexFilePath);

			// テクスチャの読み込み
			HRESULT hr = S_OK;
			hr = CreateWICTextureFromFileEx(
				D3D->GetDevice(),
				D3D->GetDeviceContext(),
				texFileName,
				0,
				D3D11_USAGE_DEFAULT,
				D3D11_BIND_SHADER_RESOURCE,
				0, 0,
				WIC_LOADER_FORCE_SRGB,
				nullptr,
				materials[matName].Texture.ReleaseAndGetAddressOf()
			);
			if (FAILED(hr))
				return hr;
		}

		return S_OK;
	}

	//--------------------------------------------------------------------------------------
	// fbx頂点情報の読み込み

	void LoadFbxVertex(FbxMesh* fMesh,
					   std::vector<MeshNode>& meshs,
					   int& materialNum)
	{
		FbxVector4* allVertPoints = fMesh->GetControlPoints();

		// UVセット名の取得
		FbxStringList uvSetNames;
		fMesh->GetUVSetNames(uvSetNames);

		// ポリゴンの取得
		int polygonCount = fMesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; i++)
		{
			// 使用マテリアルのインデックスの取得
			int matIndex = 0;
			int useMat = fMesh->GetElementMaterial()->GetIndexArray().GetAt(i);
			auto matName = fMesh->GetNode()->GetMaterial(useMat)->GetName();
			for (; matIndex < materialNum; matIndex++) {
				if (strcmp(meshs[matIndex].MaterialName, matName) == 0)
					break;
			}

			// ポリゴンを形成している頂点の取得
			int verCount = fMesh->GetPolygonSize(i);
			for (int j = 0; j < verCount; j++)
			{
				// 頂点データの作成
				Vertex vertex = {};

				// 頂点座標の取得
				int index = fMesh->GetPolygonVertex(i, j);
				vertex.Pos = {
					(float)allVertPoints[index].mData[0],
					(float)allVertPoints[index].mData[1],
					(float)allVertPoints[index].mData[2]
				};

				// 法線の取得
				FbxVector4 normal;
				fMesh->GetPolygonVertexNormal(i, j, normal);
				vertex.Normal = {
					(float)normal[0],
					(float)normal[1],
					(float)normal[2]
				};

				// UVセットがあれば設定する
				if (uvSetNames.GetCount() > 0) {
					FbxVector2 uv;
					bool is = false;
					fMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv, is);
					vertex.Texture = { (float)uv[0], 1 - (float)uv[1] };
				}
				else
					vertex.Texture = { 0.0f,0.0f };

				// 4頂点以降なら、インデックスを追加して三角形にする
				if (j >= 3) {
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 3]);
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 2]);
				}

				// 頂点、インデックスの追加
				meshs[matIndex].Vertices.emplace_back(vertex);
				meshs[matIndex].Indices.emplace_back((DWORD)(meshs[matIndex].Vertices.size() - 1));
			}
		}
	}

	//--------------------------------------------------------------------------------------
	// fbxボーン情報の読み込み

	void LoadFbxBoneWeight(FbxMesh* fMesh,
						   std::unordered_map<std::string, UINT>& bones,
						   std::unordered_map<int, BoneWeight>& boneWeights)
	{
		int skinCount = fMesh->GetDeformerCount(FbxDeformer::eSkin);

		// スキンの取得
		for (int skinIndex = 0; skinIndex < skinCount; skinIndex++) {
			FbxSkin* skin = (FbxSkin*)fMesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
			int clusterCount = skin->GetClusterCount();

			// ボーンの取得
			for (int boneIndex = 0; boneIndex < clusterCount; boneIndex++) {
				FbxCluster* cluster = skin->GetCluster(boneIndex);

				// ボーンが既に存在するか
				std::string boneName = cluster->GetName();
				auto it = bones.find(boneName);
				if (it == bones.end())
				{
					bones[boneName] = (UINT)bones.size();
				}

				// ボーンの重みの取得
				int pointCount = cluster->GetControlPointIndicesCount();
				int* indices = cluster->GetControlPointIndices();
				double* weights = cluster->GetControlPointWeights();

				for (int i = 0; i < pointCount; i++) {
					if (boneWeights[indices[i]].BoneIndex.x == 0) {
						boneWeights[indices[i]].BoneIndex.x = (float)bones[boneName];
						boneWeights[indices[i]].Weight.x = (float)weights[i];
					}
					else if (boneWeights[indices[i]].BoneIndex.y == 0) {
						boneWeights[indices[i]].BoneIndex.y = (float)bones[boneName];
						boneWeights[indices[i]].Weight.y = (float)weights[i];
					}
					else if (boneWeights[indices[i]].BoneIndex.z == 0) {
						boneWeights[indices[i]].BoneIndex.z = (float)bones[boneName];
						boneWeights[indices[i]].Weight.z = (float)weights[i];
					}
					else if (boneWeights[indices[i]].BoneIndex.w == 0) {
						boneWeights[indices[i]].BoneIndex.w = (float)bones[boneName];
						boneWeights[indices[i]].Weight.w = (float)weights[i];
					}
					else {
						// 既に４つとも埋まってしまっている
						// 最も影響が少ないボーンを取得
						int index = 0;
						float minWeight = boneWeights[indices[i]].Weight.x;
						if (boneWeights[indices[i]].Weight.y < minWeight) {
							minWeight = boneWeights[indices[i]].Weight.y;
							index = 1;
						}
						if (boneWeights[indices[i]].Weight.z < minWeight) {
							minWeight = boneWeights[indices[i]].Weight.z;
							index = 2;
						}
						if (boneWeights[indices[i]].Weight.w < minWeight)
							index = 3;

						// 影響が少なければ無視
						if (minWeight > (float)weights[i])
							continue;

						// ボーンを置き換える
						switch (index)
						{
						case 0:
							boneWeights[indices[i]].BoneIndex.x = (float)bones[boneName];
							boneWeights[indices[i]].Weight.x = (float)weights[i];
							break;

						case 1:
							boneWeights[indices[i]].BoneIndex.y = (float)bones[boneName];
							boneWeights[indices[i]].Weight.y = (float)weights[i];
							break;

						case 2:
							boneWeights[indices[i]].BoneIndex.z = (float)bones[boneName];
							boneWeights[indices[i]].Weight.z = (float)weights[i];
							break;

						case 3:
							boneWeights[indices[i]].BoneIndex.w = (float)bones[boneName];
							boneWeights[indices[i]].Weight.w = (float)weights[i];
							break;
						}
					}
				}
			}
		}
	}

	//--------------------------------------------------------------------------------------
	// fbxアニメーション頂点情報の読み込み

	void LoadFbxAnimVertex(FbxMesh* fMesh,
						   std::vector<MeshAnimNode>& meshs,
						   std::unordered_map<std::string, UINT>& bones,
						   int& materialNum)
	{
		std::unordered_map<int, BoneWeight> boneWeights;
		LoadFbxBoneWeight(fMesh, bones, boneWeights);

		FbxVector4* allVertPoints = fMesh->GetControlPoints();

		// UVセット名の取得
		FbxStringList uvSetNames;
		fMesh->GetUVSetNames(uvSetNames);

		// ポリゴンの取得
		int polygonCount = fMesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; i++)
		{
			// 使用マテリアルのインデックスの取得
			int matIndex = 0;
			int useMat = fMesh->GetElementMaterial()->GetIndexArray().GetAt(i);
			auto matName = fMesh->GetNode()->GetMaterial(useMat)->GetName();
			for (; matIndex < materialNum; matIndex++) {
				if (strcmp(meshs[matIndex].MaterialName, matName) == 0)
					break;
			}

			// ポリゴンを形成している頂点の取得
			int verCount = fMesh->GetPolygonSize(i);
			for (int j = 0; j < verCount; j++)
			{
				// 頂点データの作成
				VertexAnim vertex = {};

				// 頂点座標の取得
				int index = fMesh->GetPolygonVertex(i, j);
				vertex.Pos = {
					(float)allVertPoints[index].mData[0],
					(float)allVertPoints[index].mData[1],
					(float)allVertPoints[index].mData[2]
				};

				// 法線の取得
				FbxVector4 normal;
				fMesh->GetPolygonVertexNormal(i, j, normal);
				vertex.Normal = {
					(float)normal[0],
					(float)normal[1],
					(float)normal[2]
				};

				// UVセットがあれば設定する
				if (uvSetNames.GetCount() > 0) {
					FbxVector2 uv;
					bool is = false;
					fMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv, is);
					vertex.Texture = { (float)uv[0], 1 - (float)uv[1] };
				}
				else
					vertex.Texture = { 0.0f,0.0f };

				// ボーンの重みの取得
				vertex.BoneIndex = boneWeights[index].BoneIndex;
				vertex.Weight = boneWeights[index].Weight;

				// 4頂点以降なら、インデックスを追加して三角形にする
				if (j >= 3) {
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 3]);
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 2]);
				}

				// 頂点、インデックスの追加
				meshs[matIndex].Vertices.emplace_back(vertex);
				meshs[matIndex].Indices.emplace_back((DWORD)(meshs[matIndex].Vertices.size() - 1));
			}
		}
	}
}

HRESULT MeshImporter::LoadFbxMesh(LPCWSTR fileName,
								  std::vector<MeshNode>& meshs,
								  std::unordered_map<std::string, Material>& materials,
								  int& materialNum)
{
	bool suc = true;
	HRESULT hr = S_OK;

	char mbFileName[128] = {};
	size_t st;
	wcstombs_s(&st, mbFileName, wcslen(fileName) + 1, fileName, _TRUNCATE);

	FbxManager* fbxManager = FbxManager::Create();
	if (fbxManager == nullptr)
		return E_FAIL;

	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
	if (fbxImporter == nullptr) {
		fbxManager->Destroy();
		return E_FAIL;
	}

	FbxScene* fbxScene = FbxScene::Create(fbxManager, "");
	if (fbxScene == nullptr) {
		fbxImporter->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// ファイルオープン
	suc = fbxImporter->Initialize(mbFileName);
	if (suc == false) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// シーンにインポートする
	suc = fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();
	if (suc == false) {
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// マテリアルの読み込み
	materialNum = fbxScene->GetSrcObjectCount<FbxSurfaceMaterial>();
	meshs.resize(materialNum);
	for (int i = 0; i < materialNum; i++) {
		CHAR matName[256] = {};
		LoadFbxMaterial(fbxScene->GetSrcObject<FbxSurfaceMaterial>(i), fileName, matName, materials);
		strcpy_s(meshs[i].MaterialName, matName);
	}

	// メッシュの読み込み
	int meshNum = fbxScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshNum; i++)
		LoadFbxVertex(fbxScene->GetSrcObject<FbxMesh>(i), meshs, materialNum);

	// FBXデータの破棄
	fbxScene->Destroy();
	fbxManager->Destroy();

	CreateBuffers(meshs, materialNum);

	return S_OK;
}

HRESULT MeshImporter::LoadFbxAnimMesh(LPCWSTR fileName,
									  std::vector<MeshAnimNode>& meshs,
									  std::unordered_map<std::string, UINT>& bones,
									  std::unordered_map<std::string, Material>& materials,
									  int& materialNum)
{
	bool suc = true;
	HRESULT hr = S_OK;

	char mbFileName[128] = {};
	size_t st;
	wcstombs_s(&st, mbFileName, wcslen(fileName) + 1, fileName, _TRUNCATE);

	FbxManager* fbxManager = FbxManager::Create();
	if (fbxManager == nullptr)
		return E_FAIL;

	FbxImporter* fbxImporter = FbxImporter::Create(fbxManager, "");
	if (fbxImporter == nullptr) {
		fbxManager->Destroy();
		return E_FAIL;
	}

	FbxScene* fbxScene = FbxScene::Create(fbxManager, "");
	if (fbxScene == nullptr) {
		fbxImporter->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// ファイルオープン
	suc = fbxImporter->Initialize(mbFileName);
	if (suc == false) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// シーンにインポートする
	suc = fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();
	if (suc == false) {
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// マテリアルの読み込み
	materialNum = fbxScene->GetSrcObjectCount<FbxSurfaceMaterial>();
	meshs.resize(materialNum);
	for (int i = 0; i < materialNum; i++) {
		CHAR matName[256] = {};
		LoadFbxMaterial(fbxScene->GetSrcObject<FbxSurfaceMaterial>(i), fileName, matName, materials);
		strcpy_s(meshs[i].MaterialName, matName);
	}

	// メッシュの読み込み
	int meshNum = fbxScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshNum; i++)
		LoadFbxAnimVertex(fbxScene->GetSrcObject<FbxMesh>(i), meshs, bones, materialNum);

	// FBXデータの破棄
	fbxScene->Destroy();
	fbxManager->Destroy();

	// ボーンの重みの正規化
	for (size_t i = 0; i < meshs.size(); i++) {
		for (size_t j = 0; j < meshs[i].Vertices.size(); j++) {
			XMFLOAT4 weights = meshs[i].Vertices[j].Weight;
			float total = weights.x + weights.y + weights.z + weights.w;

			if (total > 0.0f)
			{
				meshs[i].Vertices[j].Weight = {
					weights.x / total,
					weights.y / total,
					weights.z / total,
					weights.w / total,
				};
			}
		}
	}

	auto device = D3D->GetDevice();

	for (int i = 0; i < materialNum; i++)
	{
		// 頂点バッファの作成
		D3D11_BUFFER_DESC verDesc = {};
		verDesc.Usage = D3D11_USAGE_DEFAULT;
		verDesc.ByteWidth = sizeof(VertexAnim) * (UINT)meshs[i].Vertices.size();
		verDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		verDesc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA InitData = {};
		InitData.pSysMem = meshs[i].Vertices.data();
		hr = device->CreateBuffer(&verDesc, &InitData, meshs[i].VertexBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return hr;

		// インデックスバッファの作成
		D3D11_BUFFER_DESC indexDesc = {};
		indexDesc.Usage = D3D11_USAGE_DEFAULT;
		indexDesc.ByteWidth = sizeof(DWORD) * (UINT)meshs[i].Indices.size();
		indexDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		indexDesc.CPUAccessFlags = 0;
		InitData.pSysMem = meshs[i].Indices.data();
		hr = device->CreateBuffer(&indexDesc, &InitData, meshs[i].IndexBuffer.ReleaseAndGetAddressOf());
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}