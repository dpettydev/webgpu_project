#include "core/file/file_async.h"
#include "core/file/mem_file.h"
#include "core/file/file_manager.h"
#include "core/file/file_manager.h"
#include "util/stringn.h"


drgFileAsync::drgFileAsync(drgInt32 priority)
{
	m_State = DRG_FILE_ASYNC_NOT_ASYNC;
	m_Priority = priority;
	m_Next = NULL;
	m_Callback = NULL;
	m_CallBackData = NULL;
	m_InQueue = false;
}

drgFileAsync::~drgFileAsync()
{
	m_AssetMutex.Lock();
	bool inqueue = m_InQueue;
	m_AssetMutex.Unlock();
	if (inqueue == true)
		Close();
	while (inqueue == true)
	{
		//drgThread::SleepCurrent(1);
		m_AssetMutex.Lock();
		inqueue = m_InQueue;
		m_AssetMutex.Unlock();
	}
}

bool drgFileAsync::OpenAsync(const char* filename, bool check_web)
{
	return OpenAsync(filename, DefaultFileCallback, NULL, check_web);
}

bool drgFileAsync::OpenAsync(const char* filename, drgCallback callback, void* data, bool check_web)
{
	Close();
	const char* asset_server = drgFileManager::GetAssetServer();
	bool success = false;

	char strNewName[DRG_MAX_PATH];
	check_web = (check_web && (asset_server[0] != '\0'));
	drgFileManager::UnmakeResourceName(strNewName, filename, DRG_MAX_PATH);

	m_AssetMutex.Lock();
	drgString::Copy(m_AssetName, strNewName);
	m_MemBuf = NULL;
	m_FilePtr = 0;
	m_Delete = true;
	if (check_web)
		m_State = DRG_FILE_ASYNC_OPENING_WEB;
	else
		m_State = DRG_FILE_ASYNC_OPENING;
	m_Callback = callback;
	m_CallBackData = data;
	success = true;
	drgFileManager::AddToQueue(this, false);
	m_AssetMutex.Unlock();
	return success;
}

void drgFileAsync::Close()
{
	m_AssetMutex.Lock();
	if (m_State == DRG_FILE_ASYNC_NOT_ASYNC)
	{
		drgAssetReader::Close();
	}
	else if (m_State != DRG_FILE_ASYNC_DONE)
	{
		m_State = DRG_FILE_ASYNC_CLOSEING;
		drgFileManager::AddToQueue(this, false);
	}
	m_AssetMutex.Unlock();
}

void drgFileAsync::DefaultFileCallback(void* data)
{
}

