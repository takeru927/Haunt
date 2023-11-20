#include "Animation.h"
#include "fbxsdk.h"

Animation::Animation()
	: m_Time(0.0f), m_Span(0.0f), m_CurrentFrame(0), m_StartFrame(0), m_EndFrame(0)
{
	m_FrameBones = std::make_unique<XMMATRIX[]>(128);
}

HRESULT Animation::LoadAnimation(LPCWSTR fileName, const std::unordered_map<std::string, UINT>& bones)
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
		fbxScene->Destroy();
		fbxImporter->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	// シーンにインポートする
	suc = fbxImporter->Import(fbxScene);
	if (suc == false) {
		fbxScene->Destroy();
		fbxImporter->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}

	int animCount = fbxImporter->GetAnimStackCount();
	fbxImporter->Destroy();
	if (animCount <= 0) {
		// アニメーションが存在しない
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}
	
	// アニメーション情報の取得
	// １つのアニメーションしか取得しない
	FbxArray<FbxString*> animNames = {};
	fbxScene->FillAnimStackNameArray(animNames);
	if (animNames[1]->Buffer() == nullptr) {
		// エラー
		fbxScene->Destroy();
		fbxManager->Destroy();
		return E_FAIL;
	}
	FbxTakeInfo* animInfo = fbxScene->GetTakeInfo(animNames[1]->Buffer());
	FbxTime timePerFrame = FbxTime::GetOneFrameValue(FbxTime::eFrames30);
	
	// アニメーション時間 / １フレーム時間　でフレーム数を取得する
	m_StartFrame = (UINT)(animInfo->mLocalTimeSpan.GetStart() / timePerFrame).Get();
	m_EndFrame   = (UINT)(animInfo->mLocalTimeSpan.GetStop() / timePerFrame).Get();
	m_CurrentFrame = m_StartFrame;
	m_Time = 0.0f;
	m_Span = 0.033333f;	// 30fps
	//m_Span = 0.016666f;	// 60fps

	// メモリ解放
	for (int i = 0; i < animCount; i++)
		delete animNames[i];

	// ボーンアニメーションの取得
	m_BoneTransforms.reserve(128);
	m_BoneTransforms.resize(bones.size());
	int meshCount = fbxScene->GetSrcObjectCount<FbxMesh>();
	for (int i = 0; i < meshCount; i++) {
		hr = GetMeshBones(fbxScene->GetSrcObject<FbxMesh>(i), bones);
		if (FAILED(hr)) {
			fbxScene->Destroy();
			fbxManager->Destroy();
			return hr;
		}
	}

	fbxScene->Destroy();
	fbxManager->Destroy();

	return S_OK;
}

void Animation::Update(const double& time, const float& fElapsedTime)
{
	m_Time += fElapsedTime;
	if (m_Time > m_Span)
	{
		m_Time -= m_Span;
		if (++m_CurrentFrame == m_EndFrame)
			m_CurrentFrame = m_StartFrame;
	}
}

void Animation::ResetAnimation()
{
	m_CurrentFrame = m_StartFrame;
	m_Time = 0.0f;
}

XMMATRIX* Animation::GetFrameBones()
{
	size_t boneCount = m_BoneTransforms.size();
	for (size_t boneIndex = 1; boneIndex < boneCount; boneIndex++)
	{
		m_FrameBones[boneIndex] = XMMatrixMultiplyTranspose(m_BoneTransforms[boneIndex].InvInitMat, 
															m_BoneTransforms[boneIndex].FrameMat[m_CurrentFrame]);
	}
	
	return m_FrameBones.get();
}

HRESULT Animation::GetMeshBones(FbxMesh* mesh, const std::unordered_map<std::string, UINT>& bones)
{
	int skinCount = mesh->GetDeformerCount(FbxDeformer::eSkin);

	FbxNode* node = mesh->GetNode();
	FbxAMatrix geometryTransform(node->GetGeometricTranslation(FbxNode::eSourcePivot),
								 node->GetGeometricRotation(FbxNode::eSourcePivot),
								 node->GetGeometricScaling(FbxNode::eSourcePivot));
	
	// スキンの取得
	for (int i = 0; i < skinCount; i++) {
		FbxSkin* skin = (FbxSkin*)mesh->GetDeformer(i, FbxDeformer::eSkin);
		int clusterCount = skin->GetClusterCount();
		
		// ボーンの取得
		for (int j = 0; j < clusterCount; j++) {
			FbxCluster* cluster = skin->GetCluster(j);
			
			const char* boneName = cluster->GetName();
			auto it = bones.find((std::string)boneName);
			// ボーンが違う
			if (it == bones.end())	
				return E_FAIL;
			
			// フレーム行列が空なら、ボーン情報を持っていないと判断する
			UINT boneIndex = it->second;
			if (m_BoneTransforms[boneIndex].FrameMat.empty())
			{
				strcpy_s(m_BoneTransforms[boneIndex].Name, boneName);
				
				// ボーン初期位置の逆行列の取得
				FbxAMatrix transformMat = {};
				cluster->GetTransformMatrix(transformMat);
				FbxAMatrix initMat = {};
				cluster->GetTransformLinkMatrix(initMat);

				initMat = initMat.Inverse() * transformMat * geometryTransform;
				
				const double* pMat = (const double*)initMat;
				m_BoneTransforms[boneIndex].InvInitMat = {
					(float)pMat[0] ,(float)pMat[1] ,(float)pMat[2] ,(float)pMat[3],
					(float)pMat[4] ,(float)pMat[5] ,(float)pMat[6] ,(float)pMat[7],
					(float)pMat[8] ,(float)pMat[9] ,(float)pMat[10],(float)pMat[11],
					(float)pMat[12],(float)pMat[13],(float)pMat[14],(float)pMat[15],
				};

				FbxTime timePerFrame = {};
				timePerFrame.SetTime(0, 0, 0, 1, 0, FbxTime::eFrames30);
				
				// ボーンのフレーム行列の取得
				m_BoneTransforms[boneIndex].FrameMat.resize(m_EndFrame);
				for (UINT frame = m_StartFrame; frame < m_EndFrame; frame++) {
					FbxTime time = timePerFrame * frame;

					FbxAMatrix frameMat = {};

					frameMat = node->EvaluateGlobalTransform(time) * geometryTransform;
					frameMat = frameMat.Inverse() * cluster->GetLink()->EvaluateGlobalTransform(time);

					pMat = (const double*)frameMat;
					m_BoneTransforms[boneIndex].FrameMat[frame] = {
						(float)pMat[0] ,(float)pMat[1] ,(float)pMat[2] ,(float)pMat[3],
						(float)pMat[4] ,(float)pMat[5] ,(float)pMat[6] ,(float)pMat[7],
						(float)pMat[8] ,(float)pMat[9] ,(float)pMat[10],(float)pMat[11],
						(float)pMat[12],(float)pMat[13],(float)pMat[14],(float)pMat[15],
					};
				}
			}
		}
	}

	return S_OK;
}