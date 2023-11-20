#pragma once
#include <xaudio2.h>
#include <stdio.h>

struct Chunk
{
	char id[4];
	UINT size;
};

struct  RiffHeader
{
	Chunk chunk;
	char  format[4];
};

struct FormatChunk
{
	Chunk chunk;
	WAVEFORMAT fmt;
};

class WavSound
{
private:
	WAVEFORMATEX m_Format;
	Chunk		 m_Data;
	BYTE*		 m_Audio;

public:
	WavSound();
	WavSound(WavSound&& wav) noexcept;	// ムーブコンストラクタ
	~WavSound();

	HRESULT Load(LPCWSTR fileName);

	WAVEFORMATEX* GetFormat();
	Chunk* GetData();
	BYTE* GetAudio();
};