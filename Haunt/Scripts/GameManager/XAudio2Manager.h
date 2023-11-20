#pragma once
#include <xaudio2.h>
#include <wrl/client.h>
#include <list>
#include "WavSound.h"

constexpr float FPS = 60.0f;

struct WAVE {
	WCHAR	 keyName[128];
	WavSound waveData;
	WAVE() {
		wcscpy_s(keyName, L"");
	}
};

// シングルトンクラス
class XAudio2Manager
{
private:
	XAudio2Manager();

	static XAudio2Manager* instance;

public:
	static void CreateInstance()
	{
		instance = new XAudio2Manager();
	}

	static void DeleteInstance()
	{
		delete instance;
	}

	static XAudio2Manager* GetInstance()
	{
		return instance;
	}

private:
	Microsoft::WRL::ComPtr<IXAudio2> m_XAudio2;
	IXAudio2MasteringVoice*			 m_MasteringVoice;
	IXAudio2SourceVoice*			 m_BGM;
	std::list<IXAudio2SourceVoice*>	 m_SEList;
	std::list<WAVE>					 m_WavList;

	float m_FadeVolume;
	float m_VolumeIncrement;
	bool  m_isFade;
	
public:
	~XAudio2Manager();

	void Update();
	
	void OpenWave(LPCWSTR keyName, LPCWSTR fileName);
	void Play(LPCWSTR keyName);
	void PlayBGM(LPCWSTR keyName, float volume = 1.0f, float fadeTime = 0.0f);
	void StopBGM(float fadeTime = 0.0f);

	float GetBGMVolume();
	void SetBGMVolume(float volume);
};