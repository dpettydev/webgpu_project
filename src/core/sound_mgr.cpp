#include "core/sound_mgr.h"
#include "util/rand.h"

//*******************************
//! XML Structures
//*******************************
XMLSoundAmbient::XMLSoundAmbient()
{
	drgString::Copy(m_Name, "");
	drgString::Copy(m_FileName, "");
	m_Volume = 1.0f;
}

XMLSoundGroup::XMLSoundGroup()
{
	drgString::Copy(m_Name, "");
	m_MaxPlayCount = 0;
	m_CurrentlyPlaying = 0;
	m_Volume = 1.0f;
}

XMLSoundFile::XMLSoundFile()
{
	drgString::Copy(m_FileName, "");
	m_Frequency = -1;
}

XMLSoundLump::XMLSoundLump()
{
	drgString::Copy(m_FileName, "");
	m_Frequency = -1;
	memset(&m_Sounds, 0, sizeof(XMLSoundFile *) * XML_MAX_SOUND_FILES);
	m_NumSounds = 0;
}

XMLSoundLump::~XMLSoundLump()
{
	for (int i = 0; i < m_NumSounds; ++i)
		delete m_Sounds[i];
}

XMLSoundBank::XMLSoundBank()
{
	drgString::Copy(m_Name, "");
	memset(&m_Lumps, 0, sizeof(XMLSoundLump *) * XML_MAX_SOUND_LUMPS);
	m_NumLumps = 0;

	m_LoadedLump = NULL;
	memset(&m_LoadedSounds, 0, sizeof(XMLSoundFile *) * XML_MAX_SOUND_FILES);
	m_NumLoadedSounds = 0;

	for (int i = 0; i < XML_MAX_SOUND_FILES; ++i)
	{
		m_SoundIndexes[i] = -1;
		m_Frequencies[i] = -1;
	}
	m_LastPlayedIndex = 0;
	m_StopIndex = 0;
	m_SoundGroup = NULL;
};

XMLSoundBank::~XMLSoundBank()
{
	for (int i = 0; i < m_NumLumps; ++i)
		delete m_Lumps[i];
}

int XMLSoundBank::GetRandomSound()
{
	float frequencyAccum = 0.0;
	float rand = drgRand::Randf(0.0f, 1.0f);

	// use written random numbers first
	for (int i = 0; i < m_StopIndex; ++i)
	{
		if (frequencyAccum <= rand && rand <= (m_Frequencies[i] + frequencyAccum))
			return m_SoundIndexes[i];

		frequencyAccum += m_Frequencies[i];
	}

	int remaining = m_NumLoadedSounds - 1;
	if (m_StopIndex)
		remaining = m_NumLoadedSounds - m_StopIndex;
	short randomValue = drgRand::Rand(m_StopIndex, remaining);
	return m_SoundIndexes[randomValue];
}

XMLSoundLump *XMLSoundBank::GetRandomLump()
{
	short randomValue = drgRand::Rand(0, m_NumLumps - 1);
	return m_Lumps[randomValue];
}

void XMLSoundBank::BuildSoundFrequencies()
{
	m_StopIndex = 0;
	for (int i = 0; i < m_NumLoadedSounds; ++i)
	{
		if (m_LoadedSounds[i]->m_Frequency != -1)
		{
			m_Frequencies[i] = ((float)m_LoadedSounds[i]->m_Frequency) / 100.0f;
			m_StopIndex++;
		}
		else
			m_Frequencies[i] = -1;
	}

	// check for consistency
	for (int i = 0; i < m_StopIndex; ++i)
	{
		if (m_Frequencies[i] == -1)
		{
			// didn't sort properly
			assert(0);
		}
	}
}

XMLSoundDef::XMLSoundDef()
{
	drgString::Copy(m_Name, "");
	memset(&m_Banks, 0, sizeof(XMLSoundBank *) * XML_MAX_SOUND_BANKS);
	m_NumBanks = 0;
};

XMLSoundDef::~XMLSoundDef()
{
	for (int i = 0; i < m_NumBanks; ++i)
		delete m_Banks[i];
}

//*******************************
//! drgSound
//*******************************
drgSound::drgSound()
{
	Init();
}

drgSound::~drgSound()
{
}

void drgSound::Init()
{
	m_SndBuffer = NULL;
	m_pData = NULL;
	m_Format = NULL;
	m_BufferSize = 0;
	m_IsLooping = false;
	m_IsPositional = false;
	m_PointPos.Set(0.0f, 0.0f, 0.0f);
	m_Scale = 1.0f;
	m_Volume = 1.0f;
	m_PosUpdate = 1.0f;
	m_Offset = 0.0f;
	m_VoiceNum = 0;
	m_AtemptToPlay = 0;
	m_IsUsed = false;
	m_bPaused = false;
	m_bPlaying = false;
	m_Next = NULL;

	m_Owner = NULL;
	m_ActiveListID = -1;
	m_ForcedSoundIndex = -1;
	m_SoundIndex = -1;

	m_XMLSoundBank = NULL;
	m_IsManaged = false;
	m_IsInList = false;
	m_AccruedSamples = 0;
	m_MaxSamples = 0;

	m_HDWRAudioInfo = NULL;

	for (int i = 0; i < DRG_INPUTCHANNELS * DRG_OUTPUTCHANNELS; ++i)
		m_MatrixCoefficients[i] = 1.0f;
}

void drgSound::Manage()
{
	// will be cleaned up in the next update
	m_IsManaged = true;
}

int drgSound::Play(bool loop, float volume, float frequency, float pan, DRG_SND_EFX effect)
{
	m_IsLooping = loop;

	// can't use this if we're not in use
	assert(m_IsUsed);

	// if we have a buffer, stop/destroy it and get a new one
	if (m_SndBuffer)
		Stop();

	// we shouldn't have a buffer at this point
	assert(!m_SndBuffer);

	// check to see if our group allows us to play.  if no group, play no matter what
	bool groupOK = true;
	int sndIndex = -1;

	// if no bank, skip check
	if (m_XMLSoundBank)
	{
		// get a sound index from the bank
		sndIndex = m_XMLSoundBank->GetRandomSound();
		m_SoundIndex = sndIndex;

		if (m_XMLSoundBank->m_SoundGroup)
			groupOK = m_XMLSoundBank->m_SoundGroup->CanPlay();

		// get volume from bank definition
		m_Volume = m_XMLSoundBank->m_SoundGroup->m_Volume * volume;

		// if the group says no, return
		if (!groupOK || !drgSoundMgr::CheckRef(m_SoundIndex))
			return 0;
	}
	else
	{
		// if we get here, we must be a sound explicitly loaded via lump/file and thus, have an index given to us
		assert(m_ForcedSoundIndex != -1);
		sndIndex = m_ForcedSoundIndex;
	}

	if ((sndIndex < 0) || (sndIndex >= DRG_SOUNDS_SFWARE_MAX))
	{
		drgPrintOut("Failed HDWRSet Index bad %i!\n", sndIndex);
		return 0;
	}

	// bind sound data to hardware buffer
	if (!HDWRSet(drgSoundMgr::GetSoundData(sndIndex)))
	{
		drgPrintOut("Failed HDWRSet!\n");
		return 0;
	}
	assert(m_SndBuffer);

	if (HDWRPlay())
	{
		// finally playing, add to a list
		drgSoundMgr::AddSoundToList(this);

		// increase the # of sounds playing on our group
		if (m_XMLSoundBank)
		{
			if (m_XMLSoundBank->m_SoundGroup)
			{
				m_XMLSoundBank->m_SoundGroup->IncRef();
				drgSoundMgr::IncRef(m_SoundIndex);
			}
		}

		m_bPlaying = true;
	}
	else
	{
		// sound didn't play for some reason
		drgPrintError("Failed HDWRPlay!\n");
	}

	return 0;
}

int drgSound::Play3D(drgVec3 pos, bool loop, float volume, float scale, float frequency, DRG_SND_EFX effect, bool doppler)
{
	m_Scale = scale;
	m_PointPos = pos;
	m_IsLooping = loop;
	m_IsPositional = true;
	m_PosUpdate = DRG_SOUND_UPDATE_TIME;

	//! Attempt to play
	int playResult = Play(loop, volume, frequency, 0.000000, effect);

	//! Set Matrix Coefficients?
	// HDWRUpdate();

	return 0;
}

void drgSound::Stop()
{
	if (m_SndBuffer)
	{
		if (HDWRStop())
			m_SndBuffer = NULL;

		if (m_XMLSoundBank)
		{
			if (m_XMLSoundBank->m_SoundGroup)
			{
				m_XMLSoundBank->m_SoundGroup->DecRef();

				assert(m_SoundIndex != -1);
				drgSoundMgr::DecRef(m_SoundIndex);
				m_SoundIndex = -1;
			}
		}

		if (m_IsInList)
			drgSoundMgr::RemoveSoundFromList(this);

		// if we're managed, once we stop we won't have a handle to this sound, so we must free it for use again
		if (m_IsManaged)
			m_IsUsed = false;

		m_bPlaying = false;
	}
}

void drgSound::Pause(bool pause)
{
	if (m_SndBuffer)
	{
		m_bPaused = pause;

		if (!HDWRPause())
			m_bPaused = !m_bPaused;
	}
}

void drgSound::Update()
{
	HDWRUpdate();
}

void drgSound::SetEffect(DRG_SND_EFX effect)
{
}

// void drgSound::AdjustVolume(float volume)
//{
//	m_Volume=volume;
// }

void drgSound::PanVolume(float attenuateStart, float attenuateEnd, float *volOunt, float *panOut, float *volR, float *volL)
{
	drgVec3 diff;
	float dist, vol, panVal;
	vol = 0.0f;

	diff = m_PointPos - drgSoundMgr::m_drgSoundPointFrom;
	dist = diff.Length();
	diff.Normalize();

	if (dist > attenuateStart)
	{
		vol = 1.0f - (dist - attenuateStart) * (1.0f / (attenuateEnd - attenuateStart));
		if (vol < 0.0f)
			vol = 0.0f;

		drgVec3 soundvec = drgSoundMgr::m_drgSoundVect;
		soundvec.Normalize();
		panVal = diff.Dot(soundvec) * 2.0f;
		(*volL) = vol * DRG_CLAMP((1.0f - panVal), 0.0f, 1.0f);
		(*volR) = vol * DRG_CLAMP((1.0f + panVal), 0.0f, 1.0f);
		//		drgPrintOut("PAN: %f  L:%f R:%f\r\n", panVal, (*volL), (*volR));
	}
	else
	{
		(*volL) = (*volR) = 1.0f;
	}
}

void drgSound::Aquire(XMLSoundBank *bank, bool isManaged)
{
	// cleanup everything
	m_SndBuffer = NULL;
	m_pData = NULL;
	m_Format = NULL;
	m_BufferSize = 0;
	m_IsLooping = false;
	m_IsPositional = false;
	m_PointPos.Set(0.0f, 0.0f, 0.0f);
	m_Scale = 1.0f;
	m_Volume = 1.0f;
	m_PosUpdate = 1.0f;
	m_VoiceNum = 0;
	m_AtemptToPlay = 0;
	m_bPaused = false;
	m_Next = NULL;

	m_Owner = NULL;
	m_ActiveListID = -1;
	m_ForcedSoundIndex = -1;
	m_XMLSoundBank = bank;
	m_IsManaged = isManaged;
	m_IsUsed = true;
}

void drgSound::Release()
{
	m_XMLSoundBank = NULL;
	m_IsUsed = false;
	m_IsManaged = false;
}

//*******************************
//! drgSoundMgr
//*******************************
// drgSoundMgr* drgSoundMgr::m_Instance = NULL;
bool drgSoundMgr::m_bInitialized = false;
bool drgSoundMgr::m_bHardwareInitialized = false;
float drgSoundMgr::m_fMainVol = 1.0f;
int drgSoundMgr::m_NumSoundsPlaying = 0;
int drgSoundMgr::m_NumSoundsStarted = 0;
int drgSoundMgr::m_NumSoundsStopped = 0;
DRG_SND_EFX drgSoundMgr::m_CurGlobalEffect = DRG_SND_EFX_DEFAULT;
char *drgSoundMgr::m_Sounds[DRG_SOUNDS_SFWARE_MAX];
drgSound *drgSoundMgr::m_PlayingList = NULL;
drgVec3 drgSoundMgr::m_drgSoundPointFrom;
drgVec3 drgSoundMgr::m_drgSoundVect;
void *drgSoundMgr::m_ptrDevice = NULL;
drgSound drgSoundMgr::m_SoundBuffers[DRG_SOUNDS_HDWARE_MAX];
int drgSoundMgr::m_CurrentDirectSound = 0;
int drgSoundMgr::m_DirectSoundBufferPlaycount[DRG_SOUNDS_HDWARE_MAX];
drgSound *drgSoundMgr::m_ActiveList[DRG_SOUNDS_HDWARE_MAX];
int drgSoundMgr::m_NumActive = 0;
XMLSoundAmbient *drgSoundMgr::m_XMLSoundAmbients[DRG_AMBIENT_MAX];
int drgSoundMgr::m_NumXMLSoundAmbients = 0;
XMLSoundGroup *drgSoundMgr::m_XMLSoundGroups[ENGINE_MAX_SOUND_GROUPS];
int drgSoundMgr::m_NumSoundGroups = 0;
XMLSoundDef *drgSoundMgr::m_XMLSoundDefs[ENGINE_MAX_SOUND_DEFS];
int drgSoundMgr::m_NumXMLSoundDefs = 0;
XMLSoundDef *drgSoundMgr::m_LoadedSoundDefs[ENGINE_MAX_SOUND_DEFS];
int drgSoundMgr::m_NumLoadedSoundDefs = 0;
XMLSoundBank *drgSoundMgr::m_LoadedSoundBanks[ENGINE_MAX_SOUND_BANKS];
int drgSoundMgr::m_NumLoadedSoundBanks = 0;
int drgSoundMgr::m_TotalSoundsPlayed = 0;

drgSoundMgr::drgSoundMgr()
{
	m_bInitialized = false;
	m_bHardwareInitialized = false;
}

drgSoundMgr::~drgSoundMgr()
{
}

void drgSoundMgr::Init()
{
	// for(int i = 0; i < DRG_SOUND_GROUP_MAX; ++i)
	//	m_fGroupVolumes[i] = 1.0f;

	for (int i = 0; i < DRG_SOUNDS_SFWARE_MAX; ++i)
		m_Sounds[i] = NULL;

	m_bInitialized = true;
	m_fMainVol = 1.0f;
	m_NumSoundsPlaying = 0;
	m_NumSoundsStarted = 0;
	m_NumSoundsStopped = 0;
	m_CurGlobalEffect = DRG_SND_EFX_DEFAULT;
	m_PlayingList = 0;
	m_ptrDevice = 0;
	m_CurrentDirectSound = 0;
	m_drgSoundPointFrom.Set(0.0f, 0.0f, 0.0f);
	m_drgSoundVect.Set(0.0f, 0.0f, 0.0f);

	memset(&m_ActiveList, 0, sizeof(drgSound *) * DRG_SOUNDS_HDWARE_MAX);
	m_NumActive = 0;

	memset(m_DirectSoundBufferPlaycount, 0, sizeof(int) * DRG_SOUNDS_HDWARE_MAX);

	memset(&m_XMLSoundAmbients, 0, sizeof(XMLSoundAmbient *) * DRG_AMBIENT_MAX);
	m_NumXMLSoundAmbients = 0;

	memset(&m_XMLSoundGroups, 0, sizeof(XMLSoundGroup *) * ENGINE_MAX_SOUND_GROUPS);
	m_NumSoundGroups = 0;

	memset(&m_XMLSoundDefs, 0, sizeof(XMLSoundDef *) * ENGINE_MAX_SOUND_DEFS);
	m_NumXMLSoundDefs = 0;

	memset(&m_LoadedSoundDefs, 0, sizeof(XMLSoundDef *) * ENGINE_MAX_SOUND_DEFS);
	m_NumLoadedSoundDefs = 0;

	memset(&m_LoadedSoundBanks, 0, sizeof(XMLSoundBank *) * ENGINE_MAX_SOUND_BANKS);
	m_NumLoadedSoundBanks = 0;

	for (int i = 0; i < DRG_SOUNDS_HDWARE_MAX; ++i)
		m_SoundBuffers[i].Init();

	m_TotalSoundsPlayed = 0;

	HDWRInit();
}

void drgSoundMgr::CleanUp()
{
	//! can't cleanup a second time, just return.  probably b/c we have to cleanup early on windows and got here during engine free
	if (!m_bInitialized)
		return;
	m_bInitialized = false;

	//! same for hardware
	if (!m_bHardwareInitialized)
		return;
	m_bHardwareInitialized = false;

	// for(int i = 0; i < DRG_AMBIENT_MAX; ++i)
	//{
	//	if(m_Ambients[i] && m_Ambients[i]->m_IsUsed)
	//	{
	//		m_Ambients[i]->Stop();
	//	}
	// }

	for (int i = 0; i < m_NumActive; ++i)
	{
		if (m_ActiveList[i] && m_ActiveList[i]->m_IsUsed)
		{
			m_ActiveList[i]->Stop();
		}
	}

	HDWRCleanUp();

	for (int i = 0; i < m_NumXMLSoundDefs; ++i)
		delete m_XMLSoundDefs[i];

	for (int i = 0; i < m_NumXMLSoundAmbients; ++i)
		delete m_XMLSoundAmbients[i];
}

void drgSoundMgr::Update(double dtime, drgVec3 panpos, drgVec3 panvec)
{
	// don't update unless we're initialized
	if (!m_bInitialized || !m_bHardwareInitialized)
		return;

	//! Update 3D position
	m_drgSoundPointFrom = panpos;
	m_drgSoundVect = panvec;

	//! Hardware update
	HDWRUpdate();

	drgSound *curSound = 0;
	int curIndex = 0;

	while (m_ActiveList[curIndex] && curIndex < DRG_SOUNDS_HDWARE_MAX)
	{
		// cleanup the hardware buffer if the sound is done
		if (Manage(m_ActiveList[curIndex]))
		{
			// hopefully the hardware cleaned up the buffer
			assert(!m_ActiveList[curIndex]->m_SndBuffer);
		}

		// no more sound buffer, see what we should do
		if (!m_ActiveList[curIndex]->m_SndBuffer)
		{
			// freeing a hardware buffer, so lets decriment the reference
			if (m_ActiveList[curIndex]->m_XMLSoundBank)
			{
				if (m_ActiveList[curIndex]->m_XMLSoundBank->m_SoundGroup)
				{
					m_ActiveList[curIndex]->m_XMLSoundBank->m_SoundGroup->DecRef();
					assert(m_ActiveList[curIndex]->m_SoundIndex != -1);
					DecRef(m_ActiveList[curIndex]->m_SoundIndex);
					m_ActiveList[curIndex]->m_SoundIndex = -1;
				}
			}

			m_ActiveList[curIndex]->m_bPlaying = false;

			// if our active list id is -1, we're not in the active list.. how is this possible?
			assert(m_ActiveList[curIndex]->m_ActiveListID != -1);
			// if we're a managed sound, free it for aquisition again
			if (m_ActiveList[curIndex]->m_IsManaged)
				m_ActiveList[curIndex]->Release();

			RemoveSoundFromList(m_ActiveList[curIndex]);

			// continue so we update this index again, removing replaces this index with the last ID.
			// therefore, the next iteration of this list will be a sound that hasn't been updated yet
			continue;
		}

		//! update positional sounds
		if (m_ActiveList[curIndex]->m_IsPositional)
		{
			m_ActiveList[curIndex]->Update();
		}

		++curIndex;
	}
}

void drgSoundMgr::XMLLoadSoundDefsFile(const char *szLumpName, const char *szFileName)
{
	// 	/*char szFullPath[DRG_MAX_PATH];
	// 	drgString::Copy(szFullPath, drgFile::GetResourceFolder());
	// 	drgString::Concatenate(szFullPath, szFileName);

	// 	drgXMLNode = drgXMLUtil::ReadXMLFile(szFullPath);*/

	// 	//now loading from a lump
	// 	void *resource = drgLump::FindResource(szLumpName, szFileName);
	// 	//we need to get the file size when loading an xml file from a buffer
	// 	drgDirectoryEntry *e = drgLump::Find(drgLump::Query(szLumpName), szFileName);
	// 	int size = e->size;
	// 	drgXMLNode soundMgrNode = drgXMLUtil::GetXMLNodeFromBuffer(static_cast<const char*>(resource), size);

	// 	if(soundMgrNode->IsEmpty())
	// 	{
	// 		assert(0);
	// 	}

	// 	// go through the file, grab all sound groups it holds, and load them
	// 	for(unsigned int i = 0; i < soundMgrNode->GetNumChild(); ++i)
	// 	{
	// 		if(drgString::Compare(soundMgrNode->GetChildNode(i)->GetName(), "SOUNDGROUP") == 0)
	// 			XMLLoadSoundGroup(soundMgrNode->GetChildNode(i));
	// 	}

	// 	for(unsigned int i = 0; i < soundMgrNode->GetNumChild(); ++i)
	// 	{
	// 		if(drgString::Compare(soundMgrNode->GetChildNode(i)->GetName(), "SOUNDAMBIENT") == 0)
	// 			XMLLoadSoundAmbient(soundMgrNode->GetChildNode(i));
	// 	}

	// 	// go through the file, grab all the definitions it holds, and load them
	// 	for(unsigned int i = 0; i < soundMgrNode->GetNumChild(); ++i)
	// 	{
	// 		if(drgString::Compare(soundMgrNode->GetChildNode(i)->GetName(), "SOUNDDEF") == 0)
	// 			XMLLoadSoundDef(soundMgrNode->GetChildNode(i));
	// 	}
}

void drgSoundMgr::LoadSoundDef(const char *szDefName)
{
	assert(m_NumLoadedSoundDefs < ENGINE_MAX_SOUND_DEFS);

	// check to see if we're already loaded
	for (int i = 0; i < m_NumLoadedSoundDefs; ++i)
	{
		if (drgString::Compare(m_LoadedSoundDefs[i]->m_Name, szDefName) == 0)
		{
			drgPrintOut("Sound Def Already Loaded: %s\n", szDefName);
			return;
		}
	}

	// check to see if sound def exists
	int defIndex = -1;
	for (int i = 0; i < m_NumXMLSoundDefs; ++i)
	{
		if (drgString::Compare(m_XMLSoundDefs[i]->m_Name, szDefName) == 0)
		{
			defIndex = i;
			break;
		}
	}

	// didn't find.  we need to load it
	if (defIndex == -1)
	{
		drgPrintOut("Sound Def XML Not Loaded: %s\n", szDefName);
		return;
	}

	// find an open spot to place in the loaded list
	XMLSoundDef *loadMe = m_XMLSoundDefs[defIndex];
	for (int i = 0; i < ENGINE_MAX_SOUND_DEFS; ++i)
	{
		if (!m_LoadedSoundDefs[i])
		{
			m_LoadedSoundDefs[i] = loadMe;
			++m_NumLoadedSoundDefs;
			break;
		}
	}

	for (int b = 0; b < loadMe->m_NumBanks; ++b)
	{
		XMLSoundBank *curBank = loadMe->m_Banks[b];
		LoadSoundBank(curBank);
	}
}

void drgSoundMgr::ReloadSoundDef(const char *szDefName)
{
	UnloadSoundDef(szDefName, false);
	LoadSoundDef(szDefName);
}

void drgSoundMgr::UnloadSoundDef(const char *szDefName, bool unloadLumps)
{
	// check to see if sound def exists
	bool templateExists = false;
	for (int i = 0; i < m_NumXMLSoundDefs; ++i)
	{
		if (drgString::Compare(m_XMLSoundDefs[i]->m_Name, szDefName) == 0)
		{
			templateExists = true;
			break;
		}
	}

	if (!templateExists)
	{
		drgPrintOut("Can't unload because sound Def XML File Not Loaded %s\n", szDefName);
		return;
	}

	// check to see if we're loaded
	int defIndex = -1;
	for (int i = 0; i < m_NumLoadedSoundDefs; ++i)
	{
		if (drgString::Compare(m_LoadedSoundDefs[i]->m_Name, szDefName) == 0)
		{
			defIndex = i;
			break;
		}
	}

	if (defIndex == -1)
	{
		drgPrintOut("Can't unload because sound def is Not Loaded %s\n", szDefName);
		return;
	}

	XMLSoundDef *curDef = m_LoadedSoundDefs[defIndex];

	//! TODO - rework this part of the logic.  we can still play things we're unloading for now
	// if any of our groups have a playcount, then we can't unload
	// for(int b = 0; b < curDef->m_NumBanks; ++b)
	//{
	//	XMLSoundBank* curBank = curDef->m_Banks[b];

	//	if(curBank->m_SoundGroup->m_CurrentlyPlaying > 0)
	//	{
	//		char msg[DRG_MAX_PATH];
	//		sprintf(msg, "Can't unload because sound groups used by the def are in use. %s\n", szDefName);
	//		drgDebug::Print(msg);
	//		return;
	//	}
	//}

	// finally unload stuff
	for (int b = 0; b < curDef->m_NumBanks; ++b)
	{
		XMLSoundBank *curBank = curDef->m_Banks[b];

		if (unloadLumps)
		{
			// unload lump if nothing else is using it
			// if(!IsLumpBeingUsed(curBank->m_LoadedLump->m_FileName, defIndex))
			// 	drgLump::Unload(curBank->m_LoadedLump->m_FileName);
		}

		UnLoadSoundBank(curBank);
	}

	// finally remove the def
	--m_NumLoadedSoundDefs;
	assert(m_NumLoadedSoundDefs >= 0);
	m_LoadedSoundDefs[defIndex] = m_LoadedSoundDefs[m_NumLoadedSoundDefs];
	m_LoadedSoundDefs[m_NumLoadedSoundDefs] = NULL;
}

// void drgSoundMgr::XMLLoadSoundGroup(drgXMLNode groupNode)
// {
// 	XMLSoundGroup* newGroup = new XMLSoundGroup();

// 	const char* szGroupName	= groupNode->GetAttribute("NAME");
// 	int playCount = drgString::AtoI(groupNode->GetAttribute("MAXPLAY"));
// 	float groupVolume = drgString::AtoF(groupNode->GetAttribute("VOLUME"));

// 	drgString::Copy(newGroup->m_Name, szGroupName);
// 	newGroup->m_MaxPlayCount = playCount;
// 	newGroup->m_Volume = groupVolume;

// 	// make sure we aren't loaded
// 	for(int i = 0; i < m_NumSoundGroups; ++i)
// 	{
// 		if(drgString::Compare(szGroupName, m_XMLSoundGroups[i]->m_Name) == 0)
// 		{
// 			drgPrintOut("Sound Group Already Loaded: %s\n", szGroupName);
// 			return;
// 		}
// 	}

// 	// find open sound group & add it to our global list
// 	for(int i = 0; i < ENGINE_MAX_SOUND_GROUPS; ++i)
// 	{
// 		if(!m_XMLSoundGroups[i])
// 		{
// 			m_XMLSoundGroups[i] = newGroup;
// 			++m_NumSoundGroups;
// 			break;
// 		}
// 	}
// }

// void drgSoundMgr::XMLLoadSoundAmbient(drgXMLNode ambientNode)
// {
// 	// too much!
// 	assert(m_NumXMLSoundAmbients < DRG_AMBIENT_MAX);

// 	XMLSoundAmbient* newAmbient = new XMLSoundAmbient();

// 	const char* szAmbientName = ambientNode->GetAttribute("NAME");
// 	drgString::Copy(newAmbient->m_Name, szAmbientName);

// 	const char* szAmbientFile = ambientNode->GetAttribute("FILE");
// 	drgString::Copy(newAmbient->m_FileName, szAmbientFile);

// 	float ambientVolume = drgString::AtoF(ambientNode->GetAttribute("VOLUME"));
// 	newAmbient->m_Volume = ambientVolume;

// 	for(int i = 0; i < DRG_AMBIENT_MAX; ++i)
// 	{
// 		if(!m_XMLSoundAmbients[i])
// 		{
// 			m_XMLSoundAmbients[i] = newAmbient;
// 			++m_NumXMLSoundAmbients;
// 			break;
// 		}
// 	}
// }

// void drgSoundMgr::XMLLoadSoundDef(drgXMLNode defNode)
// {
// 	XMLSoundDef* newDef = new XMLSoundDef();

// 	const char* szDefName = defNode->GetAttribute("NAME");
// 	drgString::Copy(newDef->m_Name, szDefName);

// 	// make sure we aren't loaded
// 	for(int i = 0; i < m_NumXMLSoundDefs; ++i)
// 	{
// 		if(drgString::Compare(szDefName, m_XMLSoundDefs[i]->m_Name) == 0)
// 		{
// 			drgPrintOut("Sound Def Already Loaded: %s\n", szDefName);
// 			return;
// 		}
// 	}

// 	// load our sound banks
// 	assert(defNode->GetNumChild() < XML_MAX_SOUND_BANKS);
// 	for(unsigned int i = 0; i < defNode->GetNumChild(); ++i)
// 	{
// 		if(drgString::Compare(defNode->GetChildNode(i)->GetName(), "SOUNDBANK") == 0)
// 		{
// 			XMLSoundBank* newBank = XMLLoadSoundBank(defNode->GetChildNode(i));
// 			newDef->m_Banks[newDef->m_NumBanks++] = newBank;
// 		}
// 	}

// 	// find open sound group & add it to our global list
// 	for(int i = 0; i < ENGINE_MAX_SOUND_DEFS; ++i)
// 	{
// 		if(!m_XMLSoundDefs[i])
// 		{
// 			m_XMLSoundDefs[i] = newDef;
// 			++m_NumXMLSoundDefs;
// 			break;
// 		}
// 	}
// }

// XMLSoundBank* drgSoundMgr::XMLLoadSoundBank(drgXMLNode bankNode)
// {
// 	XMLSoundBank* newBank = new XMLSoundBank();

// 	const char* szBankName = bankNode->GetAttribute("NAME");
// 	drgString::Copy(newBank->m_Name, szBankName);
// 	const char* szBankGroup = bankNode->GetAttribute("GROUP");
// 	drgString::Copy(newBank->m_GroupName, szBankGroup);

// 	//for(int i = 0; i < m_NumSoundGroups; ++i)
// 	//{
// 	//	if(drgString::Compare(newBank->m_GroupName, m_XMLSoundGroups[i]->m_Name) == 0)
// 	//	{
// 	//		newBank->m_SoundGroup = m_XMLSoundGroups[i];
// 	//		break;
// 	//	}
// 	//}
// 	//
// 	//if(!newBank->m_SoundGroup)
// 	//{
// 	//	// must have a sound group!
// 	//	// is the sound group spelled wrong?
// 	//	drgDebug::Error("Can't find sound group: %s", newBank->m_GroupName);
// 	//	assert(!newBank->m_SoundGroup);
// 	//}

// 	for(unsigned int i = 0; i < bankNode->GetNumChild(); ++i)
// 	{
// 		if(drgString::Compare(bankNode->GetChildNode(i)->GetName(), "SOUNDLUMP") == 0)
// 		{
// 			XMLSoundLump* newLump = XMLLoadSoundLump(bankNode->GetChildNode(i));
// 			newBank->m_Lumps[newBank->m_NumLumps++] = newLump;
// 		}
// 	}

// 	return newBank;
// }

// XMLSoundLump* drgSoundMgr::XMLLoadSoundLump(drgXMLNode lumpNode)
// {
// 	XMLSoundLump* newLump = new XMLSoundLump();

// 	const char* szFileName = lumpNode->GetAttribute("FILE");
// 	drgString::Copy(newLump->m_FileName, szFileName);

// 	for(unsigned int i = 0; i < lumpNode->GetNumChild(); ++i)
// 	{
// 		if(drgString::Compare(lumpNode->GetChildNode(i)->GetName(), "SOUNDFILE") == 0)
// 		{
// 			XMLSoundFile* newSound = XMLLoadSoundFile(lumpNode->GetChildNode(i));
// 			newLump->m_Sounds[newLump->m_NumSounds++] = newSound;
// 		}
// 	}

// 	return newLump;
// }

// XMLSoundFile* drgSoundMgr::XMLLoadSoundFile(drgXMLNode soundNode)
// {
// 	XMLSoundFile* newSound = new XMLSoundFile();

// 	const char* szFileName = soundNode->GetAttribute("FILE");

// 	int frequency = -1;
// 	const char* szFrequency = soundNode->GetAttribute("FREQUENCY");
// 	if(szFrequency)
// 		frequency = drgString::AtoI(szFrequency);

// 	drgString::Copy(newSound->m_FileName, szFileName);
// 	newSound->m_Frequency = frequency;

// 	return newSound;
// }

void drgSoundMgr::LoadSoundBank(XMLSoundBank *curBank)
{
	// assert(m_NumLoadedSoundBanks < ENGINE_MAX_SOUND_BANKS);

	// // find an open spot to place the bank
	// for(int i = 0; i < ENGINE_MAX_SOUND_BANKS; ++i)
	// {
	// 	if(!m_LoadedSoundBanks[i])
	// 	{
	// 		m_LoadedSoundBanks[i] = curBank;
	// 		++m_NumLoadedSoundBanks;
	// 		break;
	// 	}
	// }

	// // for now, grab just the first lump, need to figure out a cool randomize thing that works for lumps & sounds
	// // probably send just indexes, and a preset freq array and the random type to a function which returns an int.
	// XMLSoundLump* loadedLump = curBank->GetRandomLump();
	// assert(loadedLump);

	// // load the lump
	// curBank->m_LoadedLump = loadedLump;
	// drgLump::Load(loadedLump->m_FileName);

	// // associate lump sounds with the bank
	// for(int i = 0; i < loadedLump->m_NumSounds; ++i)
	// 	curBank->m_LoadedSounds[i] = loadedLump->m_Sounds[i];
	// curBank->m_NumLoadedSounds = loadedLump->m_NumSounds;

	// // grab sound indexes
	// for(int i = 0; i < curBank->m_NumLoadedSounds; ++i)
	// {
	// 	drgSoundHeader* header = static_cast<drgSoundHeader*>(drgLump::FindResource(loadedLump->m_FileName, curBank->m_LoadedSounds[i]->m_FileName));
	// 	curBank->m_SoundIndexes[i] = header->GetIndex();
	// }

	// // associate the bank with its group
	// for(int i = 0; i < m_NumSoundGroups; ++i)
	// {
	// 	if(drgString::Compare(m_XMLSoundGroups[i]->m_Name, curBank->m_GroupName) == 0)
	// 	{
	// 		curBank->m_SoundGroup = m_XMLSoundGroups[i];
	// 		break;
	// 	}
	// }

	// if(!curBank->m_SoundGroup)
	// {
	// 	drgDebug::Error("Could not find group: %s for bank: %s", curBank->m_GroupName, curBank->m_Name);
	// 	assert(0);
	// }
	// //for(int i = 0; i < m_NumLoadedSoundBanks; ++i)
	// //{
	// //	if(m_LoadedSoundBanks[i])
	// //	{
	// //		if(drgString::Compare(m_XMLSoundGroups[i]->m_Name, curBank->m_GroupName) == 0)
	// //			curBank->m_SoundGroup = m_XMLSoundGroups[i];
	// //	}
	// //}

	// curBank->BuildSoundFrequencies();
}

void drgSoundMgr::UnLoadSoundBank(XMLSoundBank *curBank)
{
	int defIndex = -1;
	for (int i = 0; i < m_NumLoadedSoundBanks; ++i)
	{
		if (m_LoadedSoundBanks[i] == curBank)
		{
			defIndex = i;
			break;
		}
	}

	assert(defIndex != -1);

	--m_NumLoadedSoundBanks;
	assert(m_NumLoadedSoundBanks >= 0);
	m_LoadedSoundBanks[defIndex] = m_LoadedSoundBanks[m_NumLoadedSoundBanks];
	m_LoadedSoundBanks[m_NumLoadedSoundBanks] = NULL;
}

bool drgSoundMgr::IsLumpBeingUsed(const char *lumpName, int defIndex)
{
	for (int i = 0; i < m_NumLoadedSoundDefs; ++i)
	{
		if (i == defIndex)
			continue;

		XMLSoundDef *curDef = m_LoadedSoundDefs[i];

		for (int b = 0; b < curDef->m_NumBanks; ++b)
		{
			XMLSoundBank *curBank = curDef->m_Banks[b];

			if (drgString::Compare(curBank->m_LoadedLump->m_FileName, lumpName) == 0)
			{
				return true;
			}
		}
	}

	return false;
}

int drgSoundMgr::GetSoundID(const char *szBankName)
{
	assert(szBankName);

	for (int i = 0; i < m_NumLoadedSoundBanks; ++i)
	{
		if (drgString::Compare(m_LoadedSoundBanks[i]->m_Name, szBankName) == 0)
			return i;
	}

	// couldn't find!
	return -1;
}

drgSound *drgSoundMgr::GetSoundByID(const int soundID)
{
	assert(soundID != -1);
	assert(soundID < ENGINE_MAX_SOUND_BANKS);
	assert(m_LoadedSoundBanks[soundID]);

	drgSound *openSound = FindOpenSound();
	if (openSound)
		openSound->Aquire(m_LoadedSoundBanks[soundID], false);
	return openSound;
}

void drgSoundMgr::PlaySoundByID(const int soundID, drgVec3 *pos)
{
	assert(soundID != -1);
	assert(soundID < ENGINE_MAX_SOUND_BANKS);
	assert(m_LoadedSoundBanks[soundID]);

	drgSound *openSound = FindOpenSound();
	assert(openSound);

	openSound->Aquire(m_LoadedSoundBanks[soundID], true);

	if (pos)
		openSound->Play3D(*pos);
	else
		openSound->Play(false, drgSoundMgr::GetMainVolume());

	// if we didn't get a sound buffer, release this sound
	if (!openSound->m_SndBuffer)
		openSound->Release();
	else
		++m_TotalSoundsPlayed;
}

drgSound *drgSoundMgr::GetSound(drgSoundHeader *header)
{
	assert(header);

	int index = header->GetIndex();

	drgSound *openSound = FindOpenSound();
	if (openSound == NULL)
		return NULL;

	openSound->Aquire(NULL, false);
	openSound->m_ForcedSoundIndex = index;
	return openSound;
}

drgSound *drgSoundMgr::GetSound(const char *lmp, const char *snd)
{
	assert(lmp);
	assert(snd);

	// drgSoundHeader* header = static_cast<drgSoundHeader*>(drgLump::FindResource(lmp, snd));
	// return GetSound(header);
}

void drgSoundMgr::PlaySound(drgSoundHeader *header, drgVec3 *pos, bool loop)
{
	assert(header);

	int index = header->GetIndex();

	drgSound *openSound = FindOpenSound();
	if (openSound == NULL)
		return;

	// drgPrintOut("PlaySound SOUND: %i", m_TotalSoundsPlayed);

	openSound->Aquire(NULL, true);
	openSound->m_ForcedSoundIndex = index;
	if (pos)
		openSound->Play3D(*pos, loop);
	else
		openSound->Play(loop);

	// if we didn't get a sound buffer, release this sound
	if (!openSound->m_SndBuffer)
		openSound->Release();
	else
		++m_TotalSoundsPlayed;
}

void drgSoundMgr::PlaySound(const char *lmp, const char *snd, drgVec3 *pos, bool loop)
{
	assert(lmp);
	assert(snd);

	// drgSoundHeader* header = static_cast<drgSoundHeader*>(drgLump::FindResource(lmp, snd));
	// PlaySound(header, pos, loop);
}

void drgSoundMgr::ReleaseSound(drgSound *snd)
{
	assert(snd);
	// remove this sound from our active list if we're in it
	if (snd->m_IsInList)
	{
		// decrement our count b/c we're pulling out of the list prior to a sound mgr update
		if (snd->m_XMLSoundBank)
		{
			if (snd->m_XMLSoundBank->m_SoundGroup)
			{
				snd->m_XMLSoundBank->m_SoundGroup->DecRef();
				assert(snd->m_SoundIndex != -1);
				DecRef(snd->m_SoundIndex);
				snd->m_SoundIndex = -1;
			}
		}

		snd->m_bPlaying = false;

		// pull from list
		RemoveSoundFromList(snd);
	}
	// finally release
	snd->Release();
}

void drgSoundMgr::AddSoundToList(drgSound *snd)
{
	assert(snd->m_ActiveListID == -1);
	assert(m_NumActive <= DRG_SOUNDS_HDWARE_MAX);
	assert(!snd->m_IsInList);
	snd->m_ActiveListID = m_NumActive;
	snd->m_IsInList = true;
	m_ActiveList[m_NumActive++] = snd;
}

void drgSoundMgr::RemoveSoundFromList(drgSound *snd)
{
	assert(snd->m_IsInList);
	int id = snd->m_ActiveListID;
	assert(id != -1);

	// swap the end with the one we're removing
	--m_NumActive;
	m_ActiveList[id] = m_ActiveList[m_NumActive];
	m_ActiveList[id]->m_ActiveListID = id;
	m_ActiveList[m_NumActive] = NULL;
	snd->m_ActiveListID = -1;
	snd->m_IsInList = false;

	int aquiredSounds = 0;
	int hardwareBuffers = 0;
	int managedSounds = 0;
	for (int i = 0; i < DRG_SOUNDS_HDWARE_MAX; ++i)
	{
		if (m_SoundBuffers[i].m_IsUsed)
		{
			++aquiredSounds;
			if (m_SoundBuffers[i].m_IsManaged)
				++managedSounds;
		}
		if (m_SoundBuffers[i].m_SndBuffer)
			++hardwareBuffers;
	}

	// drgPrintOut("Aquired Sounds (may be used by ambients):(%i/%i)\n", aquiredSounds, DRG_SOUNDS_HDWARE_MAX);
	// drgPrintOut("Managed Sounds:(%i/%i)\n", managedSounds, DRG_SOUNDS_HDWARE_MAX);
	// drgPrintOut("Aquired Hardware Buffers (may not be playing): (%i/%i)\n", hardwareBuffers, DRG_SOUNDS_HDWARE_MAX);
}

void drgSoundMgr::PlayAmbient(const char *soundFile)
{
	// assert(soundFile);
	// drgAmbientBase* curAmbient = AquireAmbient();
	// curAmbient->Start(soundFile, false);	// dont' want to loop forever since we lose our handle to it
	// ManageAmbient(curAmbient);
}

int drgSoundMgr::GetAmbientID(const char *szAmbientName)
{
	// it's ok to do a linear search in realtime if we wanted b/c there aren't many ambients
	for (int i = 0; i < DRG_AMBIENT_MAX; ++i)
	{
		if (m_XMLSoundAmbients[i])
		{
			if (drgString::Compare(m_XMLSoundAmbients[i]->m_Name, szAmbientName) == 0)
			{
				return i;
			}
		}
	}

	drgDebug::Error("Could not find ambient: %s", szAmbientName);
	return -1;
}

const char *drgSoundMgr::GetAmbientById(int id)
{
	assert(m_XMLSoundAmbients[id]);
	return m_XMLSoundAmbients[id]->m_FileName;
}

float drgSoundMgr::GetAmbientVolumeById(int id)
{
	assert(m_XMLSoundAmbients[id]);
	return m_XMLSoundAmbients[id]->m_Volume;
}

float drgSoundMgr::GetAmbientVolume(const char *szAmbientName)
{
	int ambientID = GetAmbientID(szAmbientName);
	return GetAmbientVolumeById(ambientID);
}

void drgSoundMgr::AddAmbientSoundToList(drgSound *snd)
{
	assert(!snd->m_IsUsed);
	snd->m_IsUsed = true;
}
void drgSoundMgr::RemoveAmbientSoundFromList(drgSound *snd)
{
	assert(snd->m_IsUsed);
	snd->m_IsUsed = false;
}

drgSound *drgSoundMgr::FindOpenSound()
{
	int last = m_CurrentDirectSound;
	// loop while we're used, not playing
	// while(m_SoundBuffers[m_CurrentDirectSound].IsSoundPlaying())
	while (m_SoundBuffers[m_CurrentDirectSound].GetVoiceUsed())
	{
		m_CurrentDirectSound++;

		if (m_CurrentDirectSound >= DRG_SOUNDS_HDWARE_MAX)
			m_CurrentDirectSound = 0;

		if (m_CurrentDirectSound == last)
			return NULL;
	}

	return &(m_SoundBuffers[m_CurrentDirectSound]);
}

int drgSoundMgr::GetOpenIndex()
{
	for (int i = 0; i < DRG_SOUNDS_SFWARE_MAX; i++)
		if (!m_Sounds[i])
			return i;
	drgPrintError("Failed HDWRPlay!\n");
	return -1;
}

void drgSoundMgr::PauseAll(bool pause)
{
	for (int i = 0; i < m_NumActive; ++i)
		m_ActiveList[i]->Pause(pause);
}

void drgSoundMgr::StopAll()
{
	while (m_ActiveList[0])
		m_ActiveList[0]->Stop();

	// for(int i = 0; i < m_NumActive; ++i)
	//	m_ActiveList[i]->Stop();

	// while(m_ActiveList[0])
	//	RemoveSoundFromList(0);
}

void drgSoundMgr::DrawStats()
{
	const int startingX = 15;
	const int startingY = 15;
	const int spacing = 15;
	const drgColor color(255, 255, 255, 255);

	int curX = startingX;
	int curY = startingY;

	if (!m_bInitialized || !m_bHardwareInitialized)
	{
		// drgFont::GetDefaultFont()->Print(color, curX, curY, "Sound Mgr Uninitialized");
		return;
	}

	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Sound Mgr: Total Played: %i", m_TotalSoundsPlayed);
	curY += spacing;
	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Sound Mgr Cached Definitions:%i", m_NumXMLSoundDefs);
	curY += spacing;

	int aquiredSounds = 0;
	int hardwareBuffers = 0;
	int managedSounds = 0;
	for (int i = 0; i < DRG_SOUNDS_HDWARE_MAX; ++i)
	{
		if (m_SoundBuffers[i].m_IsUsed)
		{
			++aquiredSounds;
			if (m_SoundBuffers[i].m_IsManaged)
				++managedSounds;
		}
		if (m_SoundBuffers[i].m_SndBuffer)
			++hardwareBuffers;
	}

	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Aquired Sounds (may be used by ambients):(%i/%i)", aquiredSounds, DRG_SOUNDS_MAX_BUFFERS);
	curY += spacing;

	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Aquired Ambients:(%i/%i)", aquiredAmbients, DRG_AMBIENT_MAX);
	curY += spacing;

	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Managed Sounds:(%i/%i)", managedSounds, DRG_SOUNDS_MAX_BUFFERS);
	curY += spacing;

	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Managed Ambients:(%i/%i)", m_NumManagedAmbients, DRG_AMBIENT_MAX);
	curY += spacing;

	// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Aquired Hardware Buffers (may not be playing): (%i/%i)", hardwareBuffers, DRG_SOUNDS_MAX_BUFFERS);
	curY += spacing;

	int playingSounds = 0;
	for (int i = 0; i < m_NumSoundGroups; ++i)
	{
		XMLSoundGroup *curGroup = m_XMLSoundGroups[i];

		// drgFont::GetDefaultFont()->Printf(color, curX, curY, "Group \"%s\" (%i/%i)", curGroup->m_Name, curGroup->m_CurrentlyPlaying, curGroup->m_MaxPlayCount);
		curY += spacing;
	}

	//! Draw banks
	// for(int i = 0; i < m_NumLoadedSoundDefs; ++i)
	//{
	//	XMLSoundDef* curDef = m_LoadedSoundDefs[i];

	//	for(int b = 0; b < curDef->m_NumBanks; ++b)
	//	{
	//		XMLSoundBank* curBank = curDef->m_Banks[b];

	//		sprintf(szMsgBuf, "Loaded Bank (%s).\"%s\" With Lump \"%s\"", curDef->m_Name, curBank->m_Name, curBank->m_LoadedLump->m_FileName);
	//		drgFont::GetDefaultFont()->Print(color, curX, curY, szMsgBuf);
	//		curY += spacing;
	//	}
	//}
}

void drgSoundMgr::PrintInfo()
{
	// todo - print group stats
}

void drgSoundMgr::SetGlobalEffect(DRG_SND_EFX effect)
{
}

DRG_SND_EFX drgSoundMgr::GetGlobalEffect()
{
	return DRG_SND_EFX_DEFAULT;
}

bool drgSoundMgr::IsSoundInRange(drgVec3 pos, float scale)
{
	drgVec3 diff = pos - m_drgSoundPointFrom;
	float dist = diff.Length();
	if (dist > (DRG_SOUND_ATTENUATE_END * scale))
		return false;
	return true;
}

void drgSoundMgr::IncRef(int index)
{
	assert(m_DirectSoundBufferPlaycount[index] >= 0);
	++m_DirectSoundBufferPlaycount[index];
}

void drgSoundMgr::DecRef(int index)
{
	assert(m_DirectSoundBufferPlaycount[index] != 0);
	--m_DirectSoundBufferPlaycount[index];
}

bool drgSoundMgr::CheckRef(int index)
{
	return m_DirectSoundBufferPlaycount[index] < DRG_SOUND_MAX_PLAYCOUNT;
}
