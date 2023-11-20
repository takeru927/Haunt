#include "MeshImporter.h"
#include "fbxsdk.h"
#include "GameLibrary/DirectDevice.h"

namespace
{
	// ���_�o�b�t�@�A�C���f�b�N�X�o�b�t�@�̍쐬
	HRESULT CreateBuffers(std::vector<MeshNode>& meshs, int& materialNum)
	{
		HRESULT hr = S_OK;
		auto device = D3D->GetDevice();

		for (int i = 0; i < materialNum; i++)
		{
			// ���_�o�b�t�@�̍쐬
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

			// �C���f�b�N�X�o�b�t�@�̍쐬
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
// obj�t�@�C���̃��[�h
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

	// �t�@�C���p�X�̎擾
	for (int i = lstrlenW(fileName) - 1; i > 0; --i) {
		if (fileName[i] == '/') {
			wcsncpy_s(filePath, fileName, static_cast<size_t>(i + 1));
			break;
		}
	}

	// obj�t�@�C�����J���ē��e��ǂݍ���
	FILE* fp = nullptr;
	_wfopen_s(&fp, fileName, L"r");
	if (fp == nullptr)
		return E_FAIL;

	// ���b�V���̓ǂݍ���
	while (fgets(data, dataLength, fp) != nullptr)
	{
		// �R�����g�͖�������
		if (data[0] == '#')
		{
			continue;
		}

		// �f�[�^�̐擪�A�h���X���擾����
		headPoint = strchr(data, ' ');
		if (headPoint == nullptr) continue;
		headPoint += 1;

		if (data[0] == 'v')
		{
			// ���_���W
			if (data[1] == ' ') {
				float x, y, z;
				sscanf_s(headPoint, "%f %f %f", &x, &y, &z);

				XMFLOAT3 vertex = { x,y,z };
				vertices.emplace_back(vertex);
			}

			// UV���W
			else if (data[1] == 't') {
				float x, y;
				sscanf_s(headPoint, "%f %f", &x, &y);

				y = 1.0f - y;	// Y���𔽓]������

				XMFLOAT2 texture = { x,y };
				textures.emplace_back(texture);
			}

			// �@���x�N�g��
			else if (data[1] == 'n')
			{
				float x, y, z;
				sscanf_s(headPoint, "%f %f %f", &x, &y, &z);

				XMFLOAT3 normal = { x,y,z };
				normals.emplace_back(normal);
			}
		}

		// �ʏ��
		else if (data[0] == 'f')
		{
			int face[3] = { 0 };

			// �O�p�`
			for (int i = 1;; i++) {
				// 1���_�̃f�[�^���擾����
				for (int j = 0; j < 3; j++) {
					sscanf_s(headPoint, "%d", &face[j]);

					if (j >= 2) break;	// �Ō�̃f�[�^�Ȃ�A�ړ��������ɏI������

					// ���̍��W�f�[�^�Ɉړ�������
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

				// 4���_�ȍ~�Ȃ�A�C���f�b�N�X��ǉ�����
				if (i >= 4) {
					meshs[matIndex].Indices.emplace_back((DWORD)meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 3]);
					meshs[matIndex].Indices.emplace_back((DWORD)meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 2]);
				}

				// ���_��ǉ����A���̃C���f�b�N�X���i�[
				meshs[matIndex].Vertices.emplace_back(vertex);
				meshs[matIndex].Indices.emplace_back((DWORD)meshs[matIndex].Vertices.size() - 1);

				// ���̒��_�f�[�^�Ɉړ�������
				headPoint = strchr(headPoint, ' ');
				if (headPoint == nullptr) break;	// �f�[�^���Ȃ���ΏI��
			}
		}

		// �g�p����}�e���A����
		else if (strstr(data, "usemtl") != nullptr)
		{
			sscanf_s(headPoint, "%s", currentMatName, matNameLength);

			MeshNode mesh = {};
			strcpy_s(mesh.MaterialName, currentMatName);
			meshs.emplace_back(mesh);

			matIndex++;
		}

		// mtl�t�@�C�����
		else if (strstr(data, "mtllib") != nullptr)
		{
			// mtl�t�@�C�����̃R�s�[
			WCHAR mtl[matNameLength] = {};
			sscanf_s(headPoint, "%ls", mtl, matNameLength);
			wcscpy_s(mtlFileName, filePath);
			wcscat_s(mtlFileName, mtl);
		}
	}

	// obj�t�@�C�������
	fclose(fp);
	fp = nullptr;

	// �f�[�^�擾�p�̈ꎞ�ϐ�
	CHAR matName[matNameLength] = { };

	// mtl�t�@�C�����J���āA���e��ǂݍ���
	_wfopen_s(&fp, mtlFileName, L"r");
	if (fp == nullptr)
		return E_FAIL;

	// �|�C���^��擪�ɖ߂�
	fseek(fp, SEEK_SET, 0);

	// �}�e���A���̓ǂݍ���
	while (fgets(data, dataLength, fp) != nullptr)
	{
		// �R�����g�͖�������
		if (data[0] == '#')
		{
			continue;
		}

		// �f�[�^�̐擪�A�h���X���擾����
		char* headPoint = strchr(data, ' ') + 1;
		if (headPoint == nullptr)
		{
			continue;
		}

		// �}�e���A����
		if (strstr(data, "newmtl") != nullptr)
		{
			materialNum++;
			sscanf_s(headPoint, "%s", matName, matNameLength);
		}

		else if (data[0] == 'K')
		{
			// �A���r�G���g
			if (data[1] == 'a')
			{
				float r, g, b;
				sscanf_s(headPoint, "%f %f %f", &r, &g, &b);

				XMFLOAT4 ambient = { r,g,b, 1.0f };
				materials[matName].Ambient = ambient;
			}

			// �f�B�t���[�Y
			else if (data[1] == 'd')
			{
				float r, g, b;
				sscanf_s(headPoint, "%f %f %f", &r, &g, &b);

				XMFLOAT4 diffuse = { r,g,b, 1.0f };
				materials[matName].Diffuse = diffuse;
			}

			// �G�~�b�V�u
			else if (data[1] == 'e')
			{
				float r, g, b;
				sscanf_s(headPoint, "%f %f %f", &r, &g, &b);

				XMFLOAT4 emissive = { r,g,b, 1.0f };
				materials[matName].Emissive = emissive;
			}
		}

		// �e�N�X�`��
		else if (strstr(data, "map_Kd") != nullptr)
		{
			WCHAR texName[matNameLength] = {};
			sscanf_s(headPoint, "%ls", texName, matNameLength);

			// �e�N�X�`���t�@�C���p�X�̎擾
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

	// mtl�t�@�C�������
	fclose(fp);

	CreateBuffers(meshs, materialNum);

	return S_OK;
}

//--------------------------------------------------------------------------------------
// fbx�t�@�C���̃��[�h
//--------------------------------------------------------------------------------------

namespace
{
	struct BoneWeight
	{
		XMFLOAT4 BoneIndex;
		XMFLOAT4 Weight;
	};

	//--------------------------------------------------------------------------------------
	// fbx�}�e���A���̓ǂݍ���

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

		// �e�N�X�`���̎擾
		auto prop = surfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse);
		int texCount = prop.GetSrcObjectCount<FbxFileTexture>();

		if (texCount > 0) {
			auto texture = prop.GetSrcObject<FbxFileTexture>(0);

			// �e�N�X�`���̑��΃p�X���擾
			auto mbTexFilePath = texture->GetRelativeFileName();
			wchar_t wTexFilePath[256] = {};
			for (int i = 0; i < strlen(mbTexFilePath); i++) {
				if (mbTexFilePath[i] == '\\')
					wTexFilePath[i] = '/';
				else
					wTexFilePath[i] = mbTexFilePath[i];
			}

			// ���f��������t�@�C���܂ł̃p�X���擾
			wchar_t texFileName[256] = {};
			for (size_t i = wcslen(fileName) - 1; i > 0; --i) {
				if (fileName[i] == '/') {
					wcsncpy_s(texFileName, _countof(texFileName), fileName, i + 1);
					break;
				}
			}
			// �e�N�X�`���܂ł̃t�@�C���p�X���쐬
			wcscat_s(texFileName, wTexFilePath);

			// �e�N�X�`���̓ǂݍ���
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
	// fbx���_���̓ǂݍ���

	void LoadFbxVertex(FbxMesh* fMesh,
					   std::vector<MeshNode>& meshs,
					   int& materialNum)
	{
		FbxVector4* allVertPoints = fMesh->GetControlPoints();

		// UV�Z�b�g���̎擾
		FbxStringList uvSetNames;
		fMesh->GetUVSetNames(uvSetNames);

		// �|���S���̎擾
		int polygonCount = fMesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; i++)
		{
			// �g�p�}�e���A���̃C���f�b�N�X�̎擾
			int matIndex = 0;
			int useMat = fMesh->GetElementMaterial()->GetIndexArray().GetAt(i);
			auto matName = fMesh->GetNode()->GetMaterial(useMat)->GetName();
			for (; matIndex < materialNum; matIndex++) {
				if (strcmp(meshs[matIndex].MaterialName, matName) == 0)
					break;
			}

			// �|���S�����`�����Ă��钸�_�̎擾
			int verCount = fMesh->GetPolygonSize(i);
			for (int j = 0; j < verCount; j++)
			{
				// ���_�f�[�^�̍쐬
				Vertex vertex = {};

				// ���_���W�̎擾
				int index = fMesh->GetPolygonVertex(i, j);
				vertex.Pos = {
					(float)allVertPoints[index].mData[0],
					(float)allVertPoints[index].mData[1],
					(float)allVertPoints[index].mData[2]
				};

				// �@���̎擾
				FbxVector4 normal;
				fMesh->GetPolygonVertexNormal(i, j, normal);
				vertex.Normal = {
					(float)normal[0],
					(float)normal[1],
					(float)normal[2]
				};

				// UV�Z�b�g������ΐݒ肷��
				if (uvSetNames.GetCount() > 0) {
					FbxVector2 uv;
					bool is = false;
					fMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv, is);
					vertex.Texture = { (float)uv[0], 1 - (float)uv[1] };
				}
				else
					vertex.Texture = { 0.0f,0.0f };

				// 4���_�ȍ~�Ȃ�A�C���f�b�N�X��ǉ����ĎO�p�`�ɂ���
				if (j >= 3) {
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 3]);
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 2]);
				}

				// ���_�A�C���f�b�N�X�̒ǉ�
				meshs[matIndex].Vertices.emplace_back(vertex);
				meshs[matIndex].Indices.emplace_back((DWORD)(meshs[matIndex].Vertices.size() - 1));
			}
		}
	}

	//--------------------------------------------------------------------------------------
	// fbx�{�[�����̓ǂݍ���

	void LoadFbxBoneWeight(FbxMesh* fMesh,
						   std::unordered_map<std::string, UINT>& bones,
						   std::unordered_map<int, BoneWeight>& boneWeights)
	{
		int skinCount = fMesh->GetDeformerCount(FbxDeformer::eSkin);

		// �X�L���̎擾
		for (int skinIndex = 0; skinIndex < skinCount; skinIndex++) {
			FbxSkin* skin = (FbxSkin*)fMesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
			int clusterCount = skin->GetClusterCount();

			// �{�[���̎擾
			for (int boneIndex = 0; boneIndex < clusterCount; boneIndex++) {
				FbxCluster* cluster = skin->GetCluster(boneIndex);

				// �{�[�������ɑ��݂��邩
				std::string boneName = cluster->GetName();
				auto it = bones.find(boneName);
				if (it == bones.end())
				{
					bones[boneName] = (UINT)bones.size();
				}

				// �{�[���̏d�݂̎擾
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
						// ���ɂS�Ƃ����܂��Ă��܂��Ă���
						// �ł��e�������Ȃ��{�[�����擾
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

						// �e�������Ȃ���Ζ���
						if (minWeight > (float)weights[i])
							continue;

						// �{�[����u��������
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
	// fbx�A�j���[�V�������_���̓ǂݍ���

	void LoadFbxAnimVertex(FbxMesh* fMesh,
						   std::vector<MeshAnimNode>& meshs,
						   std::unordered_map<std::string, UINT>& bones,
						   int& materialNum)
	{
		std::unordered_map<int, BoneWeight> boneWeights;
		LoadFbxBoneWeight(fMesh, bones, boneWeights);

		FbxVector4* allVertPoints = fMesh->GetControlPoints();

		// UV�Z�b�g���̎擾
		FbxStringList uvSetNames;
		fMesh->GetUVSetNames(uvSetNames);

		// �|���S���̎擾
		int polygonCount = fMesh->GetPolygonCount();
		for (int i = 0; i < polygonCount; i++)
		{
			// �g�p�}�e���A���̃C���f�b�N�X�̎擾
			int matIndex = 0;
			int useMat = fMesh->GetElementMaterial()->GetIndexArray().GetAt(i);
			auto matName = fMesh->GetNode()->GetMaterial(useMat)->GetName();
			for (; matIndex < materialNum; matIndex++) {
				if (strcmp(meshs[matIndex].MaterialName, matName) == 0)
					break;
			}

			// �|���S�����`�����Ă��钸�_�̎擾
			int verCount = fMesh->GetPolygonSize(i);
			for (int j = 0; j < verCount; j++)
			{
				// ���_�f�[�^�̍쐬
				VertexAnim vertex = {};

				// ���_���W�̎擾
				int index = fMesh->GetPolygonVertex(i, j);
				vertex.Pos = {
					(float)allVertPoints[index].mData[0],
					(float)allVertPoints[index].mData[1],
					(float)allVertPoints[index].mData[2]
				};

				// �@���̎擾
				FbxVector4 normal;
				fMesh->GetPolygonVertexNormal(i, j, normal);
				vertex.Normal = {
					(float)normal[0],
					(float)normal[1],
					(float)normal[2]
				};

				// UV�Z�b�g������ΐݒ肷��
				if (uvSetNames.GetCount() > 0) {
					FbxVector2 uv;
					bool is = false;
					fMesh->GetPolygonVertexUV(i, j, uvSetNames[0], uv, is);
					vertex.Texture = { (float)uv[0], 1 - (float)uv[1] };
				}
				else
					vertex.Texture = { 0.0f,0.0f };

				// �{�[���̏d�݂̎擾
				vertex.BoneIndex = boneWeights[index].BoneIndex;
				vertex.Weight = boneWeights[index].Weight;

				// 4���_�ȍ~�Ȃ�A�C���f�b�N�X��ǉ����ĎO�p�`�ɂ���
				if (j >= 3) {
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 3]);
					meshs[matIndex].Indices.emplace_back(meshs[matIndex].Indices[meshs[matIndex].Indices.size() - 2]);
				}

				// ���_�A�C���f�b�N�X�̒ǉ�
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

	// �t�@�C���I�[�v��
	suc = fbxImporter->Initialize(mbFileName);
	if (suc == false) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// �V�[���ɃC���|�[�g����
	suc = fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();
	if (suc == false) {
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// �}�e���A���̓ǂݍ���
	materialNum = fbxScene->GetSrcObjectCount<FbxSurfaceMaterial>();
	meshs.resize(materialNum);
	for (int i = 0; i < materialNum; i++) {
		CHAR matName[256] = {};
		LoadFbxMaterial(fbxScene->GetSrcObject<FbxSurfaceMaterial>(i), fileName, matName, materials);
		strcpy_s(meshs[i].MaterialName, matName);
	}

	// ���b�V���̓ǂݍ���
	int meshNum = fbxScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshNum; i++)
		LoadFbxVertex(fbxScene->GetSrcObject<FbxMesh>(i), meshs, materialNum);

	// FBX�f�[�^�̔j��
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

	// �t�@�C���I�[�v��
	suc = fbxImporter->Initialize(mbFileName);
	if (suc == false) {
		fbxImporter->Destroy();
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// �V�[���ɃC���|�[�g����
	suc = fbxImporter->Import(fbxScene);
	fbxImporter->Destroy();
	if (suc == false) {
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// �}�e���A���̓ǂݍ���
	materialNum = fbxScene->GetSrcObjectCount<FbxSurfaceMaterial>();
	meshs.resize(materialNum);
	for (int i = 0; i < materialNum; i++) {
		CHAR matName[256] = {};
		LoadFbxMaterial(fbxScene->GetSrcObject<FbxSurfaceMaterial>(i), fileName, matName, materials);
		strcpy_s(meshs[i].MaterialName, matName);
	}

	// ���b�V���̓ǂݍ���
	int meshNum = fbxScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshNum; i++)
		LoadFbxAnimVertex(fbxScene->GetSrcObject<FbxMesh>(i), meshs, bones, materialNum);

	// FBX�f�[�^�̔j��
	fbxScene->Destroy();
	fbxManager->Destroy();

	// �{�[���̏d�݂̐��K��
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
		// ���_�o�b�t�@�̍쐬
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

		// �C���f�b�N�X�o�b�t�@�̍쐬
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