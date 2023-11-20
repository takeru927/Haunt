#include "WavSound.h"

WavSound::WavSound()
	: m_Format{}, m_Data(), m_Audio(nullptr)
{
}

WavSound::WavSound(WavSound&& wav) noexcept
	: m_Format(wav.m_Format), m_Data(wav.m_Data), m_Audio(wav.m_Audio)
{
	wav.m_Audio = nullptr;
}

WavSound::~WavSound()
{
	delete m_Audio;
}

HRESULT WavSound::Load(LPCWSTR fileName)
{
	// ファイルオープン
	FILE* fp = nullptr;
	_wfopen_s(&fp, fileName, L"rb");
	if (fp == nullptr)
		return E_FAIL;

	// RIFFチャンクの読み込み
	RiffHeader riff;
	fread_s(&riff, sizeof(RiffHeader), sizeof(RiffHeader), 1, fp);
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0 || 
		strncmp(riff.format, "WAVE", 4) != 0)
	{
		// フォーマットエラー
		fclose(fp);
		return E_FAIL;
	}

	Chunk chunk = {};
	WAVEFORMAT format = {};

	// Chunkの読み込み
	while(fread_s(&chunk, sizeof(Chunk), sizeof(Chunk), 1, fp) != 0)
	{
		// サイズエラー
		if (chunk.size < 0) break;

		// fmt チャンクの読み込み
		if (strncmp(chunk.id, "fmt ", 4) == 0)
		{
			size_t min = min(chunk.size, sizeof(WAVEFORMAT));
			fread_s(&format, min, min, 1, fp);

			fseek(fp, chunk.size - sizeof(WAVEFORMAT), SEEK_CUR);
		}
		// dataチャンクの読み込み
		else if (strncmp(chunk.id, "data", 4) == 0)
		{
			m_Data = chunk;

			m_Audio = new BYTE[m_Data.size];
			fread_s(m_Audio, m_Data.size, m_Data.size, 1, fp);
		}
		// 認識できないデータは飛ばす
		else
		{
			fseek(fp, chunk.size, SEEK_CUR);
		}

		ZeroMemory(&chunk, sizeof(Chunk));
	}

	fclose(fp);

	// WAVEFORMATから、WAVEFORMATEXへコピーする
	memcpy(&m_Format, &format, sizeof(WAVEFORMAT));
	m_Format.wBitsPerSample = format.nBlockAlign * 8 / format.nChannels;

	return S_OK;
}

WAVEFORMATEX* WavSound::GetFormat()
{
	return &m_Format;
}

Chunk* WavSound::GetData()
{
	return &m_Data;
}

BYTE* WavSound::GetAudio()
{
	return m_Audio;
}