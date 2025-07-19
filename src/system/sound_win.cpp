///////////////////////////////////////////////////////////
//  sound_mgr.cpp
///////////////////////////////////////////////////////////

#if defined(_WINDOWS)

#include "core/sound_mgr.h"

//#include "DX/Include/xaudio2.h"
#include "engine.h"

//static drgAmbientWIN	m_Ambients[DRG_AMBIENT_MAX];

//extern HWND g_hCurDlg;

//IXAudio2*				gXAudio2 = NULL;
//IXAudio2MasteringVoice*	gMasteringVoice = NULL;


//*******************************
//! drgSound
//*******************************
void drgSound::HDWRInit()
{
}

void drgSound::HDWRCleanUp()
{
	// not implemented yet
}

bool drgSound::HDWRSet( char* dataPtr )
{
	assert(dataPtr);
	if(!dataPtr)
		return false;

	//! can't have data here
	assert(!m_SndBuffer);

	m_pData = dataPtr;

	char* soundptr=dataPtr;
	soundptr=&soundptr[sizeof(drgSoundHeader)];
	char* ptrWave = soundptr;
	void* SoundFormat=NULL;
	unsigned int SoundSize=0;

	soundptr=&soundptr[12];

	WAV_FormatChunk format;
	drgMemory::MemCopy( &format, soundptr, sizeof(WAV_FormatChunk) );
	SoundFormat=&(format.wFormatTag);
	soundptr=&soundptr[format.chunkSize+8];

	WAV_DataChunk dataHeader;
	drgMemory::MemCopy( &dataHeader, soundptr, sizeof(WAV_DataChunk) );
	soundptr=&soundptr[sizeof(WAV_DataChunk)];
	SoundSize = dataHeader.chunkSize;

	int loopCount = (m_IsLooping) ? 99999 : 0;

	//DWORD offset = (DWORD)(m_Offset * ((float)format.dwSamplesPerSec));
	if(drgSoundMgr::GetPtrDevice()==NULL)
		return false;

	//HRESULT              result;
	//LPVOID               lpAudio;
	//DWORD                dwBytes;
	//WAVEFORMATEX         wfx;

	//wfx.nAvgBytesPerSec = (format.dwSamplesPerSec*(format.wBitsPerSample/8));
	//wfx.wBitsPerSample = format.wBitsPerSample;
	//wfx.nChannels = format.wChannels;
	//wfx.nSamplesPerSec = format.dwSamplesPerSec;
	//wfx.nBlockAlign = format.wBlockAlign;
	//wfx.cbSize = SoundSize;
	//wfx.wFormatTag = WAVE_FORMAT_PCM;

	//XAUDIO2_BUFFER buffer = {0};
	//buffer.pAudioData = (BYTE*)(soundptr);
 // 	buffer.AudioBytes = SoundSize;
	//buffer.LoopCount = m_IsLooping ? XAUDIO2_LOOP_INFINITE  : 0;

	//IXAudio2SourceVoice* lpBuf;
	//result = gXAudio2->CreateSourceVoice(&lpBuf, &wfx);

	//if (FAILED(result))
	//{
	//	drgPrintOut("Failed to init sound: ERROR_SOUND_CREATEBUF!\r\n");
	//	return false;
	//}

	//result = lpBuf->SubmitSourceBuffer(&buffer);
	//if (FAILED(result))
	//{
	//	drgPrintOut("Failed to init sound: SubmitSourceBuffer!\r\n");
	//	return false;
	//}

	//m_SndBuffer = (void*)lpBuf;
	//assert(m_SndBuffer);

	return true;
}

float drgSound::GetCurrTime()
{
	return m_PosUpdate;
}

void drgSound::AdjustVolume(float volume)
{
	m_Volume=volume;
	if (!m_SndBuffer)
		return;
	//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)m_SndBuffer;
	//HRESULT result = lpBuf->SetVolume(volume, 0);
	//assert(result == S_OK);
}

bool drgSound::HDWRPlay()
{
	m_PosUpdate = 0.0f;

	//! TODO - if positional, setup matrix coefficients first
	if(m_IsPositional)
	{
		HDWRUpdate();
	}

	if (m_SndBuffer == NULL)
		return false;
	//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)m_SndBuffer;
	//lpBuf->Start(0, 0);

	return true;
}

bool drgSound::HDWRStop()
{
	if (m_SndBuffer == NULL)
		return false;
	//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)m_SndBuffer;
	//HRESULT result = lpBuf->Stop(0, 0);
	//return (result == S_OK);
	return true;
}

bool drgSound::HDWRPause()
{
	if (m_SndBuffer == NULL)
		return false;
	//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)m_SndBuffer;
	//HRESULT result = 0;
	//if (m_bPaused==true)
	//	result = lpBuf->Stop(0, 0);
	//else
	//	result = lpBuf->Start(0, 0);
	//return (result == S_OK);
	return true;
}

void drgSound::HDWRUpdate()
{
	m_PosUpdate += drgEngine::GetDeltaTime();
	if(m_IsPositional)
	{
		float panr, panl;
		PanVolume(DRG_SOUND_ATTENUATE_START * m_Scale, DRG_SOUND_ATTENUATE_END * m_Scale, NULL, NULL, &panr, &panl); 

		m_MatrixCoefficients[0] = panl * m_Volume;
		m_MatrixCoefficients[1] = panr * m_Volume;

		//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)m_SndBuffer;
		//HRESULT result = lpBuf->SetOutputMatrix( NULL, DRG_INPUTCHANNELS, DRG_OUTPUTCHANNELS, m_MatrixCoefficients, XAUDIO2_COMMIT_NOW );
		//assert(result == S_OK);
	}
}


//*******************************
//! drgSoundMgr
//*******************************
void drgSoundMgr::HDWRInit()
{
	//HRESULT result;

	//result = XAudio2Create( &gXAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	//if (FAILED(result))
	//{
	//	drgPrintOut("Failed to init XAudio2Create!\r\n");
	//	return;
	//}

	//result = gXAudio2->CreateMasteringVoice(&gMasteringVoice, XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE);
	//if (FAILED(result))
	//{
	//	drgPrintOut("Failed to init XAudio2Create!\r\n");
	//	return;
	//}

	//m_ptrDevice = (void*)gXAudio2;

	//for(int i = 0; i < DRG_AMBIENT_MAX; ++i)
	//	m_Ambients[i] = new drgAmbientWIN();

	for (int i = 0; i < DRG_SOUNDS_HDWARE_MAX; ++i)
		m_SoundBuffers[i].HDWRInit();

	m_bHardwareInitialized = true;
}

void drgSoundMgr::HDWRCleanUp()
{
	//if (gMasteringVoice != NULL)
	//{
	//	gMasteringVoice->DestroyVoice();
	//	gMasteringVoice = NULL;
	//}
	//	
	//if (gXAudio2 != NULL)
	//{
	//	gXAudio2->Release();
	//	gXAudio2 = NULL;
	//}
}

void drgSoundMgr::HDWRUpdate()
{
	//! Update streams
}

int drgSoundMgr::LoadSound(drgSoundHeader* header)
{
	// Create the sound
	int index = GetOpenIndex();
	assert(index!=-1);

	m_Sounds[index] = (char*)header;
	header->SetIndex(index);
	header->SetBuffer(m_Sounds[index]);

	return index;	// return the index this sound is sitting at.
}

void drgSoundMgr::UnLoadSound( drgSoundHeader* header )
{
	int index = header->GetIndex();
	for (int i = 0; i < DRG_SOUNDS_HDWARE_MAX; ++i)
	{
		if (m_SoundBuffers[i].m_ForcedSoundIndex == index)
		{
			m_SoundBuffers[i].HDWRStop();
			//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)m_SoundBuffers[i].m_SndBuffer;
			//if (lpBuf != NULL)
			//	lpBuf->DestroyVoice();
			m_SoundBuffers[i].m_SndBuffer = NULL;
		}
	}
	m_Sounds[index] = NULL;
}

bool drgSoundMgr::Manage(drgSound* curSound)
{
	// this cleans up the hardware buffer it is done
	if (curSound->m_SndBuffer == NULL)
		return false;
	//IXAudio2SourceVoice* lpBuf = (IXAudio2SourceVoice*)curSound->m_SndBuffer;

	//XAUDIO2_VOICE_STATE state;
	//lpBuf->GetState(&state);

	// if we're stopped and not paused, kill our hardware buffer
	//if((state.BuffersQueued <= 0))
	//{
	//	lpBuf->DestroyVoice();	
	//	curSound->m_SndBuffer = NULL;
	//	return true;	
	//}

	return false;
}

#endif //_WINDOWS
