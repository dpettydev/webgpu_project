#include "core/file/asset_read.h"
#include "core/file/file_manager.h"
#include "util/stringn.h"



drgAssetReader::drgAssetReader()
{
	m_AssetName[0] = '\0';
	m_MemBuf = NULL;
	m_FileSize = 0;
	m_FilePtr = 0;
	m_Delete = false;
}

drgAssetReader::~drgAssetReader()
{
	if (m_Delete && (m_MemBuf != NULL))
		drgMemFree(m_MemBuf);
}

bool drgAssetReader::OpenFile(const char* filename, bool check_web)
{
	Close();
	// const char* asset_server = drgFileManager::GetAssetServer();

	char strResName[DRG_MAX_PATH];
	// check_web = (check_web && (asset_server[0] != '\0'));
	drgFileManager::UnmakeResourceName(strResName, filename, DRG_MAX_PATH);

	// if (check_web)
	// {
	// 	drgFileAsync async_file(10);
	// 	async_file.OpenAsync(filename, true);
	// 	while (!async_file.IsReady())
	// 		drgThread::SleepCurrent(1);

	// 	m_FileSize = async_file.GetFileSize();
	// 	m_MemBuf = async_file.TakeOverFileMem();
	// 	async_file.Close();

	// 	m_AssetMutex.Lock();
	// 	drgString::Copy(m_AssetName, strResName);
	// 	m_FilePtr = 0;
	// 	m_Delete = true;
	// 	m_AssetMutex.Unlock();
	// }
	// else
	{
		m_AssetMutex.Lock();
		drgString::Copy(m_AssetName, strResName);
		char strNewName[DRG_MAX_PATH];
		drgString::PrintFormated(strNewName, "%s%s", drgFile::GetResourceFolder(), strResName);
		m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &m_FileSize);
		if (m_MemBuf == NULL)
		{
			drgString::PrintFormated(strNewName, "%s", strResName);
			m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &m_FileSize);
		}
		m_FilePtr = 0;
		m_Delete = true;
		m_AssetMutex.Unlock();
	}
	return (m_MemBuf != NULL);
}

bool drgAssetReader::OpenBuff(void* buff, int bufflen)
{
	Close();
	m_AssetMutex.Lock();
	drgString::Copy(m_AssetName, "DATA_ASSET");
	m_MemBuf = (unsigned char*)buff;
	m_FileSize = bufflen;
	m_FilePtr = 0;
	m_Delete = false;
	m_AssetMutex.Unlock();
	return (m_MemBuf != NULL);
}

bool drgAssetReader::OpenBuff(int bufflen)
{
	Close();
	m_AssetMutex.Lock();
	drgString::Copy(m_AssetName, "DATA_ASSET");
	m_MemBuf = (unsigned char*)drgMemAlloc(bufflen);
	m_FileSize = bufflen;
	m_FilePtr = 0;
	m_Delete = false;
	m_AssetMutex.Unlock();
	return (m_MemBuf != NULL);
}

void drgAssetReader::Close()
{
	m_AssetMutex.Lock();
	if (m_Delete && (m_MemBuf != NULL))
		drgMemFree(m_MemBuf);
	m_MemBuf = NULL;
	m_FileSize = 0;
	m_FilePtr = 0;
	m_Delete = false;
	m_AssetMutex.Unlock();
}

bool drgAssetReader::IsGood()
{
	return (m_MemBuf!=NULL);
}

const char* drgAssetReader::GetName()
{
	return m_AssetName;
}

unsigned int drgAssetReader::GetFileSize()
{
	m_AssetMutex.Lock();
	unsigned int ret = m_FileSize;
	m_AssetMutex.Unlock();
	return ret;
}

unsigned int drgAssetReader::GetFilePos()
{
	m_AssetMutex.Lock();
	unsigned int ret = m_FilePtr;
	m_AssetMutex.Unlock();
	return ret;
}

unsigned char* drgAssetReader::GetFileMem()
{
	return m_MemBuf;
}

unsigned char* drgAssetReader::TakeOverFileMem()
{
	m_Delete = false;
	return m_MemBuf;
}

bool drgAssetReader::IsEndOfFile()
{
	m_AssetMutex.Lock();
	bool ret = (m_FilePtr >= m_FileSize);
	m_AssetMutex.Unlock();
	return ret;
}

void drgAssetReader::Seek(int offset, drgFile::DRG_FILE_SEEK from)
{
	m_AssetMutex.Lock();
	if (from == drgFile::DRG_SEEK_BEGIN)
		m_FilePtr = offset;
	else if (from == drgFile::DRG_SEEK_CUR)
		m_FilePtr += offset;
	else if (from == drgFile::DRG_SEEK_END)
		m_FilePtr = (m_FileSize + offset);
	m_AssetMutex.Unlock();
}

unsigned int drgAssetReader::Read(void* out, unsigned int size)
{
	m_AssetMutex.Lock();
	if ((m_FileSize - m_FilePtr) < size)
		size = (m_FileSize - m_FilePtr);
	drgMemory::MemCopy(out, &m_MemBuf[m_FilePtr], size);
	m_FilePtr += size;
	m_AssetMutex.Unlock();
	return size;
}

unsigned int drgAssetReader::ReadEndian8(void* out)
{
	return Read(out, 1);
}

unsigned int drgAssetReader::ReadEndian16(void* out)
{
	unsigned char* getdata = (unsigned char*)out;
	unsigned int amnt = Read(getdata, 2);
	if (drgFile::GetEndianRead())
	{
		char tempval = getdata[0];
		getdata[0] = getdata[1];
		getdata[1] = tempval;
	}
	return amnt;
}

unsigned int drgAssetReader::ReadEndian32(void* out)
{
	unsigned char* getdata = (unsigned char*)out;
	unsigned int amnt = Read(getdata, 4);
	if (drgFile::GetEndianRead())
	{
		char tempval = getdata[0];
		getdata[0] = getdata[3];
		getdata[3] = tempval;
		tempval = getdata[1];
		getdata[1] = getdata[2];
		getdata[2] = tempval;
	}
	return amnt;
}

string8 drgAssetReader::ReadLine(unsigned int* _length)
{
	unsigned int count = 0;
	string8 retval;
	unsigned char getch;
	m_AssetMutex.Lock();
	while (m_FilePtr < m_FileSize)
	{
		getch = m_MemBuf[m_FilePtr];
		if (getch == '\r' || getch == '\n')
		{
			while ((m_FilePtr < m_FileSize) && (m_MemBuf[m_FilePtr] == '\r' || m_MemBuf[m_FilePtr] == '\n')) {
				m_FilePtr++;
			}
			break;
		}
		retval += getch;
		count++;
		m_FilePtr++;
	}
	m_AssetMutex.Unlock();
	if (_length != NULL) {
		(*_length) = count;
	}
	return retval;
}

