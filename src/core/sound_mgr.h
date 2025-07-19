#ifndef __DRG_SOUNDMGR_H__
#define __DRG_SOUNDMGR_H__

#include "util/math_def.h"
#include "system/file.h"
#include "util/stringn.h"
#include "core/object.h"

//*******************************
//! Defines
//*******************************
#define DRG_SOUNDS_SFWARE_MAX (300)
#if (defined(_ANDROID) || defined(_IPHONE))
#define DRG_SOUNDS_HDWARE_MAX (16)
#else
#define DRG_SOUNDS_HDWARE_MAX (300)
#endif

#define DRG_AMBIENT_MAX (8)
#define DRG_SOUND_ATTENUATE_START (5 * 12) // *12 to convert to feet
#define DRG_SOUND_ATTENUATE_END (100 * 12) // fall off over this range
#define DRG_SOUND_UPDATE_TIME (0.2f)
#define DRG_SOUND_MAX_PLAYCOUNT (16) //! # of times the same .wav can be played at the same time

#define DRG_WAV_SAS_CH (0)
#define DRG_WAV_STR_CH (1)
#define DRG_WAV_AMB_CH (2)

#define DRG_INPUTCHANNELS (1)
#define DRG_OUTPUTCHANNELS (8)

enum DRG_SND_EFX
{
	DRG_SND_EFX_DEFAULT = 0,
	DRG_SND_EFX_GENERIC,
	DRG_SND_EFX_PADDEDCELL,
	DRG_SND_EFX_ROOM,
	DRG_SND_EFX_BATHROOM,
	DRG_SND_EFX_LIVINGROOM,
	DRG_SND_EFX_STONEROOM,
	DRG_SND_EFX_AUDITORIUM,
	DRG_SND_EFX_CONCERTHALL,
	DRG_SND_EFX_CAVE,
	DRG_SND_EFX_ARENA,
	DRG_SND_EFX_HANGAR,
	DRG_SND_EFX_CARPETEDHALLWAY,
	DRG_SND_EFX_HALLWAY,
	DRG_SND_EFX_STONECORRIDOR,
	DRG_SND_EFX_ALLEY,
	DRG_SND_EFX_FOREST,
	DRG_SND_EFX_CITY,
	DRG_SND_EFX_MOUNTAINS,
	DRG_SND_EFX_QUARRY,
	DRG_SND_EFX_PLAIN,
	DRG_SND_EFX_PARKINGLOT,
	DRG_SND_EFX_SEWERPIPE,
	DRG_SND_EFX_UNDERWATER,
	DRG_SND_EFX_SMALLROOM,
	DRG_SND_EFX_MEDIUMROOM,
	DRG_SND_EFX_LARGEROOM,
	DRG_SND_EFX_MEDIUMHALL,
	DRG_SND_EFX_LARGEHALL,
	DRG_SND_EFX_PLATE,
	DRG_SND_EFX_NUM_OF_EFX,
	DRG_SND_EFX_NO_EFFECT = -1
};

//*******************************
//! Engine Structures
//*******************************

// VAG header structure
typedef struct _vagHeader
{
	unsigned char ID[4];
	unsigned int version;
	unsigned char reserved1[4];
	unsigned int dataSize;
	unsigned int sampleFreq;
	unsigned char reserved2[12];
	unsigned char name[16];
} Vag_Format;

typedef struct
{
	char chunkID[4];
	long chunkSize;

	short wFormatTag;
	unsigned short wChannels;
	unsigned long dwSamplesPerSec;
	unsigned long dwAvgBytesPerSec;
	unsigned short wBlockAlign;
	unsigned short wBitsPerSample;
} WAV_FormatChunk;

typedef struct
{
	char chunkID[4];
	long chunkSize;
} WAV_DataChunk;

//*******************************
//! XML Structures
//*******************************
// # of banks we can have per definition
#define XML_MAX_SOUND_BANKS (64)

// # of lumps per bank we can randomize
#define XML_MAX_SOUND_LUMPS (8)

// # of files in a lump
#define XML_MAX_SOUND_FILES (64)

// # of groups that the manager cares about
#define ENGINE_MAX_SOUND_GROUPS (64)

// # of sound definitions the manager cares about
#define ENGINE_MAX_SOUND_DEFS (32)

// # of banks that the manager has loaded
#define ENGINE_MAX_SOUND_BANKS (ENGINE_MAX_SOUND_DEFS * XML_MAX_SOUND_BANKS)

class XMLSoundAmbient
{
public:
	XMLSoundAmbient();

	char m_Name[DRG_MAX_PATH];
	char m_FileName[DRG_MAX_PATH];
	float m_Volume;
};

class XMLSoundGroup
{
public:
	XMLSoundGroup();

	char m_Name[DRG_MAX_PATH];
	unsigned int m_MaxPlayCount;
	unsigned int m_CurrentlyPlaying;
	float m_Volume;

	inline void ClearRef() { m_CurrentlyPlaying = 0; };
	inline void IncRef() { ++m_CurrentlyPlaying; };
	inline void DecRef() { --m_CurrentlyPlaying; };
	inline bool CanPlay() { return m_CurrentlyPlaying < m_MaxPlayCount; };
	inline float GetVolume() { return m_Volume; }
};

class XMLSoundFile
{
public:
	XMLSoundFile();

	char m_FileName[DRG_MAX_PATH];
	int m_Frequency;
};

class XMLSoundLump
{
public:
	XMLSoundLump();
	~XMLSoundLump();

	char m_FileName[DRG_MAX_PATH];
	int m_Frequency;
	XMLSoundFile *m_Sounds[XML_MAX_SOUND_FILES];
	int m_NumSounds;
};

class XMLSoundBank
{
public:
	XMLSoundBank();
	~XMLSoundBank();

	int GetRandomSound();

	XMLSoundLump *GetRandomLump();

	void BuildSoundFrequencies();

	char m_Name[DRG_MAX_PATH];
	XMLSoundLump *m_Lumps[XML_MAX_SOUND_LUMPS];
	int m_NumLumps;
	char m_GroupName[DRG_MAX_PATH];

	// pointers to sound files that we've loaded... we may want to move these into a different class
	XMLSoundLump *m_LoadedLump;
	XMLSoundFile *m_LoadedSounds[XML_MAX_SOUND_FILES];
	int m_NumLoadedSounds;

	// index to loaded sounds raw sound data
	int m_SoundIndexes[XML_MAX_SOUND_FILES];
	int m_LastPlayedIndex;
	int m_StopIndex;
	float m_Frequencies[XML_MAX_SOUND_FILES];

	// group this bank belongs to
	XMLSoundGroup *m_SoundGroup;
};

class XMLSoundDef
{
public:
	XMLSoundDef();
	~XMLSoundDef();

	char m_Name[DRG_MAX_PATH];
	XMLSoundBank *m_Banks[XML_MAX_SOUND_BANKS];
	int m_NumBanks;
};

//*******************************
//! drgSoundHeader
//*******************************
class drgSoundHeader
{
public:
	drgSoundHeader() {};
	~drgSoundHeader() {};

	//! Set/Get
	inline int GetIndex() { return m_SoundIndex; };
	inline void SetIndex(int index) { m_SoundIndex = index; };
	inline unsigned int GetSize() { return m_Size; };
	inline void SetSize(unsigned int size) { m_Size = size; };
	inline void SetBuffer(void *buff) { m_SoundBuffer = buff; };

private:
	int m_SoundIndex;
	void *m_SoundBuffer;
	unsigned int m_Size;
};

//*******************************
//! drgSound
//*******************************
class drgSoundMgr;
class drgSound
{
public:
	drgSound();
	virtual ~drgSound();

	//*******************************
	//! Cross Platform
	//*******************************
public:
	void Init();
	void Manage();
	void Release();
	int Play(bool loop = false,
			 float volume = 1.0f,
			 float frequency = 1.0f,
			 float pan = 0.0f,
			 DRG_SND_EFX effect = DRG_SND_EFX_NO_EFFECT);

	int Play3D(drgVec3 pos,
			   bool loop = false,
			   float volume = 1.0f,
			   float scale = 1.0f,
			   float frequency = 1.0f,
			   DRG_SND_EFX effect = DRG_SND_EFX_NO_EFFECT,
			   bool doppler = true);

	void Stop();
	void Pause(bool pause);
	void Update();
	float GetCurrTime();

	void AdjustVolume(float volume);
	void PanVolume(float attenuateStart, float attenuateEnd, float *volOunt, float *panOut, float *volR, float *volL);

	void SetEffect(DRG_SND_EFX effect);
	inline bool IsPlaying() { return m_bPlaying; };
	inline bool IsPaused() { return m_bPaused; };
	inline float GetVolume() { return m_Volume; };
	inline int GetVoiceNum() { return m_VoiceNum; };
	inline bool GetVoiceUsed() { return m_IsUsed; };
	inline bool IsVoicePositional() { return m_IsPositional; };
	inline bool IsVoiceLooping() { return m_IsLooping; };
	inline bool IsVoiceActive() { return m_IsInList; };
	inline void SetVoiceLooping(bool looping) { m_IsLooping = looping; }
	inline void SetVoiceUsed(bool val) { m_IsUsed = val; };
	inline void SetPosition(drgVec3 pos) { m_PointPos = pos; };
	inline void SetScale(float scale) { m_Scale = scale; };
	inline void SetTimeOffset(float offset) { m_Offset = offset; };
	inline void SetOwner(drgObjectProp *owner) { m_Owner = owner; };
	inline drgObjectProp *GetOwner() { return m_Owner; };
	inline void *GetHardwareBuffer() { return m_SndBuffer; };
	inline void **GetHardwareBufferPtr() { return &m_SndBuffer; };

protected:
	void Aquire(XMLSoundBank *bank, bool isManaged);

	//*******************************
	//! Hardware Specific
	//*******************************
protected:
	void HDWRInit();
	void HDWRCleanUp();
	bool HDWRPlay();
	bool HDWRPlay3D();
	bool HDWRStop();
	bool HDWRPause();
	void HDWRUpdate();

	bool HDWRSet(char *dataPtr);

	//*******************************
	//! Data members
	//*******************************
protected:
	//! This is our ptr to the hardware sound
	void *m_SndBuffer;

	//! This is our drgSoundHeader, or offset from the header.
	void *m_pData;

	void *m_Format;
	unsigned int m_BufferSize;
	bool m_IsLooping;
	bool m_IsPositional;
	drgVec3 m_PointPos;
	float m_Scale;
	float m_Offset;
	float m_Volume;
	float m_PosUpdate;
	float m_MatrixCoefficients[DRG_INPUTCHANNELS * DRG_OUTPUTCHANNELS];
	int m_VoiceNum;
	int m_AtemptToPlay;
	bool m_IsUsed;
	bool m_bPaused;
	bool m_bPlaying;
	drgSound *m_Next;

	unsigned long m_AccruedSamples;
	unsigned long m_MaxSamples;

	//! This is the object that this sound follows.
	drgObjectProp *m_Owner;

	//! This is the ID this sound holds in the active list in soundMgr.  Used so when we remove, we can quickly find our sound without doing a linear search.
	int m_ActiveListID;
	bool m_IsInList;

	//! XML Data
	XMLSoundBank *m_XMLSoundBank;

	//! If true, we should remove from list when we no longer have a sound buffer.  Otherwise, we wait for the user to do so.
	bool m_IsManaged;

	//! This is the index that we force if we're explicitly loaded from a lump via filename.
	int m_ForcedSoundIndex;

	int m_SoundIndex;

	void *m_HDWRAudioInfo;

	friend class drgSoundCallback;
	friend class drgSoundMgr;
};

//*******************************
//! drgSoundMgr
//*******************************
class drgSoundMgr
{
	//*******************************
	//! Singleton
	//*******************************
private:
	drgSoundMgr();
	~drgSoundMgr();

	//*******************************
	//! Cross Platform
	//*******************************
public:
	static void Init();
	static void CleanUp();

	//! XML Load/Unload
	static void XMLLoadSoundDefsFile(const char *szLumpName, const char *szFileName);
	static void LoadSoundDef(const char *szDefName);
	static void ReloadSoundDef(const char *szDefName);
	static void UnloadSoundDef(const char *szDefName, bool unloadLumps = true);

	//! Sound Aquisition
	static int GetSoundID(const char *szBankName);
	static drgSound *GetSoundByID(const int soundID);
	static void PlaySoundByID(const int soundID, drgVec3 *pos = NULL);
	static drgSound *GetSound(drgSoundHeader *header);
	static void PlaySound(drgSoundHeader *header, drgVec3 *pos = NULL, bool loop = false);
	static drgSound *GetSound(const char *lmp, const char *snd);
	static void PlaySound(const char *lmp, const char *snd, drgVec3 *pos = NULL, bool loop = false);
	static void ReleaseSound(drgSound *snd);

	static void PlayAmbient(const char *soundFile);

	static int GetAmbientID(const char *szAmbientName);
	static const char *GetAmbientById(int id);
	static float GetAmbientVolumeById(int id);
	static float GetAmbientVolume(const char *szAmbientName);

	static void Update(double dtime, drgVec3 panpos, drgVec3 panvec);
	static void PauseAll(bool pause = true);
	static void StopAll();

	static bool IsSoundInRange(drgVec3 pos, float scale);

	//! Keep track of the # of times each particular buffer is playing.
	static void IncRef(int index);
	static void DecRef(int index);
	static bool CheckRef(int index);

	//! Debug
	static void DrawStats();
	static void PrintInfo();

	//! Set/Get
	static void SetGlobalEffect(DRG_SND_EFX effect);
	static DRG_SND_EFX GetGlobalEffect();
	inline static void SetMainVolume(float volume) { m_fMainVol = DRG_CLAMP(volume, 0.0f, 1.0f); };
	inline static float GetMainVolume() { return m_fMainVol; };
	inline static void *GetPtrDevice() { return m_ptrDevice; };
	inline static char *GetSoundData(int index) { return m_Sounds[index]; };

protected:
	// static void XMLLoadSoundGroup(drgXMLNode groupNode);
	// static void XMLLoadSoundAmbient(drgXMLNode ambientNode);
	// static void XMLLoadSoundDef(drgXMLNode defNode);
	// static XMLSoundBank *XMLLoadSoundBank(drgXMLNode bankNode);
	// static XMLSoundLump *XMLLoadSoundLump(drgXMLNode lumpNode);
	// static XMLSoundFile *XMLLoadSoundFile(drgXMLNode soundNode);
	static void LoadSoundBank(XMLSoundBank *curBank);
	static void UnLoadSoundBank(XMLSoundBank *curBank);
	static bool IsLumpBeingUsed(const char *lumpName, int defIndex);
	static void AddSoundToList(drgSound *snd);
	static void RemoveSoundFromList(drgSound *snd);

	static void AddAmbientSoundToList(drgSound *snd);
	static void RemoveAmbientSoundFromList(drgSound *snd);

	static drgSound *FindOpenSound();
	static int GetOpenIndex();

	//*******************************
	//! Hardware Specific
	//*******************************
public:
	//! Clean up hardware specifics
	static void HDWRCleanUp();
	static void HDWRUpdate();

	//! Must be called after initialization
	static void HDWRInit();

	//! Sound load/unload
	static int LoadSound(drgSoundHeader *header);
	static void UnLoadSound(drgSoundHeader *header);

	static bool Manage(drgSound *curSound);

	//*******************************
	//! Data members
	//*******************************
protected:
	static bool m_bInitialized;
	static bool m_bHardwareInitialized;
	static float m_fMainVol;
	static int m_NumSoundsPlaying;
	static int m_NumSoundsStarted;
	static int m_NumSoundsStopped;
	static DRG_SND_EFX m_CurGlobalEffect;
	static char *m_Sounds[DRG_SOUNDS_SFWARE_MAX];
	static drgSound *m_PlayingList;
	static drgVec3 m_drgSoundPointFrom;
	static drgVec3 m_drgSoundVect;

	//! Ptr to hardware specific sound implementation.
	static void *m_ptrDevice;

	//! Maximum sounds that we can give the user.
	static drgSound m_SoundBuffers[DRG_SOUNDS_HDWARE_MAX];
	static int m_CurrentDirectSound;

	//! Keep track of the # of times each particular buffer is playing.  So not to play too many at once.
	static int m_DirectSoundBufferPlaycount[DRG_SOUNDS_HDWARE_MAX];

	//! List holding currently active sounds.  Used to automatically update positions.
	static drgSound *m_ActiveList[DRG_SOUNDS_HDWARE_MAX];
	static int m_NumActive;

	//! XML Template data to copy from
	static XMLSoundAmbient *m_XMLSoundAmbients[DRG_AMBIENT_MAX];
	static int m_NumXMLSoundAmbients;

	static XMLSoundGroup *m_XMLSoundGroups[ENGINE_MAX_SOUND_GROUPS];
	static int m_NumSoundGroups;

	static XMLSoundDef *m_XMLSoundDefs[ENGINE_MAX_SOUND_DEFS];
	static int m_NumXMLSoundDefs;

	//! Managed Sound Data that's currently loaded
	static XMLSoundDef *m_LoadedSoundDefs[ENGINE_MAX_SOUND_DEFS];
	static int m_NumLoadedSoundDefs;

	static XMLSoundBank *m_LoadedSoundBanks[ENGINE_MAX_SOUND_BANKS];
	static int m_NumLoadedSoundBanks;

	static int m_TotalSoundsPlayed;

	//*******************************
	//! Friends!
	//*******************************
	friend class drgSound;
};

#endif // __DRG_SOUNDMGR_H__
