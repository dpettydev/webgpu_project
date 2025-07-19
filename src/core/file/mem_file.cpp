#include "mem_file.h"

#include "util/stringn.h"

bool drgMemFile::m_BigendianRead=false;

drgMemFile::drgMemFile()
{
	Initialize();
}

drgMemFile::drgMemFile(const char* fullyQualifiedFilename, bool writeOnlyMode)
{
	Initialize();

	Open(fullyQualifiedFilename, writeOnlyMode, drgFile::DRG_SEEK_BEGIN);
}

drgMemFile::drgMemFile(void* buffer, int bufflen, bool freeBufferUponClose)
{
	Initialize();

	Open(buffer, bufflen, freeBufferUponClose, drgFile::DRG_SEEK_BEGIN);
}

drgMemFile::~drgMemFile()
{
	Close();
}

bool drgMemFile::Open(const char* fullyQualifiedFilename, bool writeOnlyMode, drgFile::DRG_FILE_SEEK from)
{
	if(m_IsOpen)
		return false;	

	if(DRG_MEM_FILE_TYPE_BUFFER == m_Type) //cannot change type
		return false;

	m_Type = DRG_MEM_FILE_TYPE_FILE;	
	m_WriteOnlyMode = writeOnlyMode;	
	SetFilename(fullyQualifiedFilename);	

	m_IsOpen = true;
	UpdateFromLatestFile();
	Seek(0, from);

	return true;
}

bool drgMemFile::Open(void* buffer, drgUInt32 bufflen, bool freeBufferUponClose, drgFile::DRG_FILE_SEEK from)
{		
	if(m_IsOpen)
		return false;

	if(DRG_MEM_FILE_TYPE_FILE == m_Type) //cannot change type
		return false;

	m_Type = DRG_MEM_FILE_TYPE_BUFFER;

	m_MemBuf = (unsigned char*)buffer;
	m_FileSize = bufflen;
	m_DeleteBuf = freeBufferUponClose;	

	m_IsOpen = true;
	Seek(0, from);

	return true;
}

drgUInt32 drgMemFile::UpdateFromLatestFile()
{
	assert(m_IsOpen);

	if(DRG_MEM_FILE_TYPE_FILE != m_Type)
		return 0;

	//if(drgFile::IsLatestVersion(m_Filename)) //doesn't work
	//	return 0;
	
	FreeMemBuff();
	
	if(m_WriteOnlyMode)
	{
		m_MemBuf = NULL;   // do not need to read the file, nor allocate any space yet.
		m_FileSize = 0;
	}
	else
	{	
		assert(NULL != m_pFilename && NULL != m_pFilename->c_str());
		m_MemBuf = (unsigned char*)drgFile::MallocFile( m_pFilename->c_str(), &m_FileSize ); //allocates, reads the file into m_MemBuf, and sets m_FileSize
		if(NULL == m_MemBuf)
			m_FileSize = 0;
	}
	
	return m_FileSize;
}

bool drgMemFile::IsGood()
{
	return !!m_MemBuf;
}

bool drgMemFile::IsEndOfFile()
{
	return (m_FilePtr>=m_FileSize);
}

unsigned int drgMemFile::GetFileSize()
{
	return m_FileSize;
}

void* drgMemFile::GetMemBuff()
{
	return m_MemBuf;
}

bool drgMemFile::Seek( drgInt32 offset, drgFile::DRG_FILE_SEEK from )
{
	assert(m_IsOpen);

	if(from==drgFile::DRG_SEEK_BEGIN)
		m_FilePtr = offset;
	else if(from==drgFile::DRG_SEEK_CUR)
		m_FilePtr += offset;
	else if(from==drgFile::DRG_SEEK_END)
		m_FilePtr = (m_FileSize+offset);

	return true;
}

drgUInt32 drgMemFile::GetCurrentFilePosition()
{
	return m_FilePtr;
}

drgUInt32 drgMemFile::Read( void* out, drgUInt32 length)
{
	assert(m_IsOpen);

	if(m_WriteOnlyMode)
		return 0;

	if(NULL == m_MemBuf && (0 == m_FileSize))
		return 0;

	assert(NULL != m_MemBuf);

	if(IsEndOfFile())
		return 0;
	
	drgUInt32 fileRemainder = (m_FileSize - m_FilePtr);
	drgUInt32 readLength = fileRemainder < length ? fileRemainder : length;
	assert(0 < readLength);

	if(out!=NULL)
		drgMemory::MemCopy(out, &m_MemBuf[m_FilePtr], readLength);		//read
	m_FilePtr += readLength;
	
	return readLength;
}

drgUInt32 drgMemFile::ReadEndian8( void* out )
{
	drgUInt32 numBytes = 1;
	numBytes = Read(out,numBytes);
	return numBytes;
}

drgUInt32 drgMemFile::ReadEndian16( void* out )
{
	drgUInt32 numBytes = 2;
	unsigned char* getdata = (unsigned char*)out;
	numBytes = Read(getdata,numBytes);
	if(m_BigendianRead)
	{
		char tempval=getdata[0];
		getdata[0]=getdata[1];
		getdata[1]=tempval;
	}
	return numBytes;
}

drgUInt32 drgMemFile::ReadEndian32( void* out )
{
	drgUInt32 numBytes = 4;
	unsigned char* getdata = (unsigned char*)out;
	numBytes = Read(getdata,numBytes);
	if(m_BigendianRead)
	{
		char tempval=getdata[0];
		getdata[0]=getdata[3];
		getdata[3]=tempval;
		tempval=getdata[1];
		getdata[1]=getdata[2];
		getdata[2]=tempval;
	}
	return numBytes;
}

drgUInt32 drgMemFile::Write( void* input, drgUInt32 length, bool force, bool truncateFile)
{	
	assert(m_IsOpen);

	drgUInt32 writeEnd = m_FilePtr + length;
	
	//resizing
	if(writeEnd > m_FileSize)
	{			
		if(DRG_MEM_FILE_TYPE_FILE == m_Type)
			ResizeBuffer(writeEnd);
		else	
			return 0;										//external buffers should never be resized
	}
	else if(truncateFile && writeEnd < m_FileSize)
	{
		if(DRG_MEM_FILE_TYPE_FILE == m_Type)
			ResizeBuffer(writeEnd);
		else if(DRG_MEM_FILE_TYPE_BUFFER == m_Type)
			memset((m_MemBuf + writeEnd), '\0', (m_FileSize - writeEnd));
		else
			assert("Unexpected DRG_MEM_FILE_TYPE");
	}	

	drgMemory::MemCopy(&m_MemBuf[m_FilePtr], input, length); //write
	m_FilePtr += length;
		
	m_ForceWrite = force ? true : m_ForceWrite;
	m_PendingUnsyncedWrites = true;
	
	return length;
}

drgUInt32 drgMemFile::WriteStr(char* input, bool force, bool truncateFile)
{
	return Write(input, drgString::Length(input), force, truncateFile);
}

drgUInt32 drgMemFile::Append( void* input, drgUInt32 length, bool force, bool truncateFile)
{
	Seek(0, drgFile::DRG_SEEK_END);
	return Write(input, length, force, truncateFile);
}

drgUInt32 drgMemFile::SyncWritesToFile()
{
	assert(m_IsOpen);

	if(DRG_MEM_FILE_TYPE_FILE != m_Type)
		return 0;

	if(false == m_PendingUnsyncedWrites)
		return 0;
	
	assert(NULL != m_MemBuf);
	assert(NULL != m_pFilename && NULL != m_pFilename->c_str());

	drgFile::DRG_FILE_MODE mode = m_ForceWrite ? drgFile::DRG_MODE_FORCE_WRITE_BIN : drgFile::DRG_MODE_WRITE_BIN;	
	drgUInt32 bytesWritten = 0;

	MEM_INFO_SET_NEW;
	drgFile* file = new drgFile();
	
	assert(NULL != file);

	bool opened = file->Open(m_pFilename->c_str(), mode);
	{		
		assert(opened);

		file->Seek(0);
		bytesWritten = file->Write(m_MemBuf, m_FileSize);
		m_PendingUnsyncedWrites = false;
	}
	file->Close();
	
	delete(file);
	
	return bytesWritten;
}

bool drgMemFile::Close()
{
	if(false == m_IsOpen)
		return false;
	
	SyncWritesToFile();
	
	if(DRG_MEM_FILE_TYPE_FILE == m_Type && NULL != m_pFilename)
	{
		delete (m_pFilename);
		m_pFilename = NULL;
	}
		
	FreeMemBuff();
	
	m_PendingUnsyncedWrites = false;
	m_ForceWrite = false;
	m_WriteOnlyMode = false; 

	m_IsOpen = false;
	return true;
}

bool drgMemFile::Clear()
{
	return ResizeBuffer(1) && SyncWritesToFile();
}

// *** PROTECTED FUNCTIONS ***	

void drgMemFile::Initialize()
{	
	m_MemBuf = NULL;
	m_FilePtr = 0;
	m_FileSize = 0;
	m_DeleteBuf = false;
	m_IsOpen = false;
	m_PendingUnsyncedWrites = false;
	m_ForceWrite = false;
	m_pFilename = NULL;
	m_WriteOnlyMode = false; 
	m_Type = DRG_MEM_FILE_TYPE_NONE;

	assert(NULL == m_MemBuf);
}

drgUInt32 drgMemFile::ResizeBuffer(drgUInt32 newLength)
{
	if(DRG_MEM_FILE_TYPE_BUFFER == m_Type)
		return m_FileSize;

	unsigned char* newBufferPointer = static_cast<unsigned char*>( drgMemReAlloc(m_MemBuf, newLength) );
	assert(NULL != newBufferPointer);
	m_MemBuf = newBufferPointer;

	if(newLength > m_FileSize)
		memset((m_MemBuf + m_FileSize), '\0', (newLength - m_FileSize));
		
	m_FileSize = newLength;
	return m_FileSize;
}

void drgMemFile::SetFilename(const char* fullyQualifiedFilename)
{
	if(DRG_MEM_FILE_TYPE_FILE != m_Type)
		return;

	assert(NULL != fullyQualifiedFilename);

	size_t uriLength = strlen(fullyQualifiedFilename) + 1;

	assert(0 < uriLength); 

	m_pFilename = new string8(uriLength, fullyQualifiedFilename);
	assert(NULL != m_pFilename);
}

bool drgMemFile::FreeMemBuff()
{
	if(m_DeleteBuf || DRG_MEM_FILE_TYPE_FILE == m_Type)
	{
		if(m_MemBuf!=NULL)
		{
			drgMemFree(m_MemBuf);
			m_MemBuf = NULL;
			m_FileSize = 0;
			return true;
		}
	}
	return false;
}
