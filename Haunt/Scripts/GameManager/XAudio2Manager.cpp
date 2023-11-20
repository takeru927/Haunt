#include "XAudio2Manager.h"
#include <stdio.h>

XAudio2Manager* XAudio2Manager::instance = nullptr;

// privateコンストラクタ
XAudio2Manager::XAudio2Manager()
	: m_XAudio2(nullptr), m_MasteringVoice(nullptr), m_BGM(nullptr)
	, m_FadeVolume(0.0f), m_VolumeIncrement(0.0f), m_isFade(false)
{
	// XAudio2の作成
	HRESULT hr = XAudio2Create(m_XAudio2.ReleaseAndGetAddressOf());
	if (FAILED(hr)) {
		CoUninitialize();
		return;
	}

	// マスターボイスの作成
	hr = m_XAudio2->CreateMasteringVoice(&m_MasteringVoice);
	if (FAILED(hr)) {
		m_XAudio2.Reset();
		CoUninitialize();
		return;
	}
}

XAudio2Manager::~XAudio2Manager()
{
	auto it = m_SEList.begin();
	while (it != m_SEList.end()){
		(*it)->Stop();
		(*it)->DestroyVoice();
		it = m_SEList.erase(it);
	}

	if (m_BGM)
		m_BGM->DestroyVoice();
	if (m_MasteringVoice)
		m_MasteringVoice->DestroyVoice();
}

void XAudio2Manager::Update()
{
	if (m_isFade)
	{
		float volume = GetBGMVolume();
		volume += m_VolumeIncrement;

		if (volume >= m_FadeVolume) volume = m_FadeVolume;

		SetBGMVolume(volume);

		if (volume <= 0.0f || volume >= m_FadeVolume)
			m_isFade = false;
	}

	auto it = m_SEList.begin();
	while (it != m_SEList.end())
	{
		XAUDIO2_VOICE_STATE state;
		(*it)->GetState(&state);
		// 再生終了していれば解放
		if (state.BuffersQueued <= 0) {
			(*it)->Stop();
			(*it)->DestroyVoice();
			it = m_SEList.erase(it);
		}
		else
			++it;
	}
}

void XAudio2Manager::OpenWave(LPCWSTR keyName, LPCWSTR fileName)
{
	// キーがリストに存在すれば、追加しない
	auto it = m_WavList.begin();
	while (it != m_WavList.end()) {
		if (wcscmp((*it).keyName, keyName) == 0) {
			return;
		}
		it++;
	}

	// wavファイルのロード
	WAVE wav;
	wcscpy_s(wav.keyName, keyName);
	wav.waveData.Load(fileName);

	m_WavList.emplace_back(std::move(wav));
}

void XAudio2Manager::Play(LPCWSTR keyName)
{
	// リストから検索する
	bool found = false;
	auto it = m_WavList.begin();
	while (it != m_WavList.end()) {
		if (wcscmp((*it).keyName, keyName) == 0) {
			found = true;
			break;
		}
		it++;
	}
	// リストに存在しない
	if (found == false)
		return;
	
	// ソースボイスの作成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT hr = m_XAudio2->CreateSourceVoice(&pSourceVoice, (*it).waveData.GetFormat());
	if (FAILED(hr))
		return;

	// 再生する波形データの登録
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (*it).waveData.GetAudio();
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = (*it).waveData.GetData()->size;
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	if (FAILED(hr)) {
		pSourceVoice->DestroyVoice();
		return;
	}
	
	// 波形データの再生
	m_SEList.emplace_back(pSourceVoice);
	pSourceVoice->Start();
}

void XAudio2Manager::PlayBGM(LPCWSTR keyName, float volume, float fadeTime)
{
	// リストから検索する
	bool found = false;
	auto it = m_WavList.begin();
	while (it != m_WavList.end()) {
		if (wcscmp((*it).keyName, keyName) == 0) {
			found = true;
			break;
		}
		it++;
	}
	// リストに存在しない
	if (found == false)
		return;

	StopBGM();

	// ソースボイスの作成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	HRESULT hr = m_XAudio2->CreateSourceVoice(&pSourceVoice, (*it).waveData.GetFormat());
	if (FAILED(hr))
		return;

	// 再生する波形データの登録
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = (*it).waveData.GetAudio();
	buf.Flags = XAUDIO2_END_OF_STREAM;
	buf.AudioBytes = (*it).waveData.GetData()->size;
	buf.LoopCount = XAUDIO2_LOOP_INFINITE;
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	if (FAILED(hr)) {
		pSourceVoice->DestroyVoice();
		return;
	}
	
	// 波形データの再生
	m_BGM = pSourceVoice;
	m_BGM->Start();
	SetBGMVolume(volume);

	if (fadeTime > 0.0f)
	{
		m_isFade = true;
		m_VolumeIncrement = volume / (fadeTime * FPS);
		m_FadeVolume = volume;

		m_BGM->SetVolume(0.0f);
	}
}

void XAudio2Manager::StopBGM(float fadeTime)
{
	if (m_BGM)
	{
		if (fadeTime > 0.0f)
		{
			m_isFade = true;
			m_VolumeIncrement = -fadeTime / FPS;
		}
		else 
		{
			m_BGM->Stop();
			m_BGM->DestroyVoice();
			m_BGM = nullptr;
		}
	}
}

float XAudio2Manager::GetBGMVolume()
{
	float volume;
	m_BGM->GetVolume(&volume);

	return volume;
}

void XAudio2Manager::SetBGMVolume(float volume)
{
	if (volume > XAUDIO2_MAX_VOLUME_LEVEL)
		volume = XAUDIO2_MAX_VOLUME_LEVEL;
	else if (volume < 0.0f)
		volume = 0.0f;

	m_BGM->SetVolume(volume);
}