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
	// �t�@�C���I�[�v��
	FILE* fp = nullptr;
	_wfopen_s(&fp, fileName, L"rb");
	if (fp == nullptr)
		return E_FAIL;

	// RIFF�`�����N�̓ǂݍ���
	RiffHeader riff;
	fread_s(&riff, sizeof(RiffHeader), sizeof(RiffHeader), 1, fp);
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0 || 
		strncmp(riff.format, "WAVE", 4) != 0)
	{
		// �t�H�[�}�b�g�G���[
		fclose(fp);
		return E_FAIL;
	}

	Chunk chunk = {};
	WAVEFORMAT format = {};

	// Chunk�̓ǂݍ���
	while(fread_s(&chunk, sizeof(Chunk), sizeof(Chunk), 1, fp) != 0)
	{
		// �T�C�Y�G���[
		if (chunk.size < 0) break;

		// fmt �`�����N�̓ǂݍ���
		if (strncmp(chunk.id, "fmt ", 4) == 0)
		{
			size_t min = min(chunk.size, sizeof(WAVEFORMAT));
			fread_s(&format, min, min, 1, fp);

			fseek(fp, chunk.size - sizeof(WAVEFORMAT), SEEK_CUR);
		}
		// data�`�����N�̓ǂݍ���
		else if (strncmp(chunk.id, "data", 4) == 0)
		{
			m_Data = chunk;

			m_Audio = new BYTE[m_Data.size];
			fread_s(m_Audio, m_Data.size, m_Data.size, 1, fp);
		}
		// �F���ł��Ȃ��f�[�^�͔�΂�
		else
		{
			fseek(fp, chunk.size, SEEK_CUR);
		}

		ZeroMemory(&chunk, sizeof(Chunk));
	}

	fclose(fp);

	// WAVEFORMAT����AWAVEFORMATEX�փR�s�[����
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