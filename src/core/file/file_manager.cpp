#include "core/file/file_manager.h"
#include "core/file/asset_read.h"
//#include "file/file_catalog.h"

#include "system/mutex.h"
#include "system/engine.h"
#include "util/priority_queue.h"
#include "util/stringn.h"
#include "system/global.h"


drgFileManager* drgFileManager::m_Instance = NULL;
//drgThread* drgFileManager::m_AssetThread = NULL;
drgMutex drgFileManager::m_FileManagerMutex;
bool drgFileManager::m_Initialized = false;
//drgFileCatalog drgFileManager::m_WebCatalog;
//drgFileAsync drgFileManager::m_WebCatalogFile(1);

char drgFileManager::m_CacheFolder[DRG_MAX_PATH] = "";
char drgFileManager::m_CacheFolderBase[DRG_MAX_PATH] = "";
char drgFileManager::m_ResourceFolder[DRG_MAX_PATH] = "";
char drgFileManager::m_ResourceFolderBase[DRG_MAX_PATH] = "";
char drgFileManager::m_AssetServerBase[512] = "";
char drgFileManager::m_AssetServer[512] = "";

drgFileManager::drgFileManager()
{
	m_RootNodeIdle = NULL;
	m_RootNodeActive = NULL;
	m_IsRunning = false;
}

drgFileManager::~drgFileManager()
{
}

void drgFileManager::Init()
{
	if (true == m_Initialized)
		return;

	drgPrintOut("drgFileManager::Initialize\r\n");

	m_FileManagerMutex.Lock();
	if (NULL == m_Instance)
	{
		//drgPrintOut("drgFileManager::StartThread\r\n");
		MEM_INFO_SET_NEW;
		m_Instance = new drgFileManager();
		// m_AssetThread = drgThread::CreateNewThread("ASSET_THREAD", m_Instance);
		// if (m_AssetThread == NULL)
		// {
		// 	delete m_Instance;
		// 	m_Instance = NULL;
		// 	drgPrintError("drgFileManager::Initialize failed\r\n");
		// 	return;
		// }
		// while (m_Instance->m_IsRunning == false)
		// 	drgThread::SleepCurrent(1);
		m_Initialized = true;
	}
	m_FileManagerMutex.Unlock();
}

void drgFileManager::Update()
{
	static int countdown = 1000;
	countdown--;
	if (countdown == 0)
	{
		//drgPrintOut("drgFileManager::Update\r\n");
		//drgFileCatalog::GetFileCatalog()->UpdateChanged();
		countdown = 1000;
	}
}

void drgFileManager::CleanUp()
{
	// drgFileCatalog::GetFileCatalog()->UpdateChanged();
	// drgFileCatalog::GetFileCatalog()->CleanUp();
	// m_WebCatalog.CleanUp();

	if (false == m_Initialized)
		return;

	drgPrintOut("drgFileManager::CleanUp\r\n");

	// m_FileManagerMutex.Lock();
	// if (NULL != m_Instance)
	// 	m_Instance->m_IsRunning = false;
	// m_FileManagerMutex.Unlock();
	// while (m_Initialized == true)
	// 	drgThread::SleepCurrent(1);
	if (NULL != m_Instance)
		delete m_Instance;
}

 void drgFileManager::AddToQueue(drgFileAsync* file, bool needs_lock)
 {
	if (m_Instance == NULL)
		return;
	if (file == NULL)
		return;

	if (needs_lock)
		file->m_AssetMutex.Lock();
	RemoveFromQueue(file);
	file->m_Next = NULL;
	file->m_InQueue = true;
	if (needs_lock)
		file->m_AssetMutex.Unlock();

	// m_FileManagerMutex.Lock();
	// if ((file->m_State == DRG_FILE_ASYNC_OPENING) || (file->m_State == DRG_FILE_ASYNC_OPENING_CACHE) || (file->m_State == DRG_FILE_ASYNC_OPENING_WEB) || (file->m_State == DRG_FILE_ASYNC_CLOSEING))
	// {
	// 	if ((m_Instance->m_RootNodeActive == NULL) || (m_Instance->m_RootNodeActive->m_Priority > file->m_Priority))
	// 	{
	// 		file->m_Next = m_Instance->m_RootNodeActive;
	// 		m_Instance->m_RootNodeActive = file;
	// 	}
	// 	else
	// 	{
	// 		volatile drgFileAsync* itr = m_Instance->m_RootNodeActive;
	// 		while (itr)
	// 		{
	// 			if ((itr->m_Next == NULL) || (itr->m_Next->m_Priority < file->m_Priority))
	// 			{
	// 				file->m_Next = itr->m_Next;
	// 				itr->m_Next = file;
	// 				itr = NULL;
	// 			}
	// 			else
	// 			{
	// 				itr = itr->m_Next;
	// 			}
	// 		}
	// 	}
	// }
	// else
	// {
	// 	file->m_Next = m_Instance->m_RootNodeIdle;
	// 	m_Instance->m_RootNodeIdle = file;
	// }
	// m_FileManagerMutex.Unlock();
}

void drgFileManager::RemoveFromQueue(drgFileAsync* file)
{
	if (m_Instance == NULL)
		return;
	if (file == NULL)
		return;

	// m_FileManagerMutex.Lock();
	// file->m_InQueue = false;
	// if (m_Instance->m_RootNodeIdle != NULL)
	// {
	// 	if (m_Instance->m_RootNodeIdle == file)
	// 	{
	// 		m_Instance->m_RootNodeIdle = m_Instance->m_RootNodeIdle->m_Next;
	// 		file->m_Next = NULL;
	// 	}
	// 	else
	// 	{
	// 		volatile drgFileAsync* prev = m_Instance->m_RootNodeIdle;
	// 		volatile drgFileAsync* itr = prev->m_Next;
	// 		while (itr)
	// 		{
	// 			if (itr == file)
	// 			{
	// 				prev->m_Next = itr->m_Next;
	// 				file->m_Next = NULL;
	// 				itr = NULL;
	// 			}
	// 			else
	// 			{
	// 				prev = itr;
	// 				itr = itr->m_Next;
	// 			}
	// 		}
	// 	}
	// }
	// if (m_Instance->m_RootNodeActive != NULL)
	// {
	// 	if (m_Instance->m_RootNodeActive == file)
	// 	{
	// 		m_Instance->m_RootNodeActive = m_Instance->m_RootNodeActive->m_Next;
	// 		file->m_Next = NULL;
	// 	}
	// 	else
	// 	{
	// 		volatile drgFileAsync* prev = m_Instance->m_RootNodeActive;
	// 		volatile drgFileAsync* itr = prev->m_Next;
	// 		while (itr)
	// 		{
	// 			if (itr == file)
	// 			{
	// 				prev->m_Next = itr->m_Next;
	// 				file->m_Next = NULL;
	// 				itr = NULL;
	// 			}
	// 			else
	// 			{
	// 				prev = itr;
	// 				itr = itr->m_Next;
	// 			}
	// 		}
	// 	}
	// }
	// m_FileManagerMutex.Unlock();
}

//overridden
// unsigned int drgFileManager::ThreadRun(drgThread* thread)
// {
// 	drgPrintOut("Started drgFileManager thread\r\n");

// 	m_IsRunning = true;
// 	while (m_IsRunning)
// 	{
// 		m_FileManagerMutex.Lock();
// 		drgFileAsync* itr = m_RootNodeActive;
// 		if (itr!=NULL)
// 		{
// 			m_RootNodeActive = itr->m_Next;
// 			itr->m_Next = NULL;
// 		}
// 		m_FileManagerMutex.Unlock();

// 		if (itr)
// 		{
// 			if ((itr->m_State == DRG_FILE_ASYNC_OPENING) || (itr->m_State == DRG_FILE_ASYNC_OPENING_CACHE))
// 			{
// 				char strNewName[DRG_MAX_PATH];
// 				if (itr->m_State == DRG_FILE_ASYNC_OPENING_CACHE)
// 				{
// 					drgString::PrintFormated(strNewName, "%s%s", GetCacheFolder(), itr->m_AssetName);
// 					itr->m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &itr->m_FileSize);
// 					if (itr->m_MemBuf == NULL)
// 					{
// 						drgString::PrintFormated(strNewName, "%s%s", drgFile::GetResourceFolder(), itr->m_AssetName);
// 						itr->m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &itr->m_FileSize);
// 						if (itr->m_MemBuf == NULL)
// 						{
// 							drgString::PrintFormated(strNewName, "%s", itr->m_AssetName);
// 							itr->m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &itr->m_FileSize);
// 						}
// 					}
// 					itr->m_Delete = true;
// 				}
// 				else
// 				{
// 					drgString::PrintFormated(strNewName, "%s%s", drgFile::GetResourceFolder(), itr->m_AssetName);
// 					itr->m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &itr->m_FileSize);
// 					if (itr->m_MemBuf == NULL)
// 					{
// 						drgString::PrintFormated(strNewName, "%s", itr->m_AssetName);
// 						itr->m_MemBuf = (unsigned char*)drgFile::MallocFile(strNewName, &itr->m_FileSize);
// 					}
// 					itr->m_Delete = true;
// 				}
// 				itr->m_FilePtr = 0;
// 				itr->m_State = DRG_FILE_ASYNC_OPENED;
// 				if (itr->m_Callback != NULL)
// 					itr->m_Callback(itr->m_CallBackData);
// 				if (itr->m_State == DRG_FILE_ASYNC_OPENED)
// 					itr->m_State = DRG_FILE_ASYNC_READY;
// 				AddToQueue(itr, true);
// 			}
// 			else if (itr->m_State == DRG_FILE_ASYNC_OPENING_WEB)
// 			{
// 				if (drgFileCatalog::GetFileCatalog()->CompareAsset(&m_WebCatalog, itr->m_AssetName) == true)
// 				{
// 					itr->m_State = DRG_FILE_ASYNC_OPENING_CACHE;
// 				}
// 				else
// 				{
// 					char strNewName[512];
// 					drgString::PrintFormated(strNewName, "%s%s", drgFileManager::GetAssetServer(), itr->m_AssetName);
// 					m_WebRequest.Init("GET", strNewName, false);
// 					m_WebRequest.Send();
// 					if ((m_WebRequest.GetStatus() == 200) && (m_WebRequest.GetResponseLength() > 0))
// 					{
// 						itr->m_FileSize = m_WebRequest.GetResponseLength();
// 						itr->m_MemBuf = (unsigned char*)drgMemAlloc(itr->m_FileSize);
// 						drgMemory::MemCopy(itr->m_MemBuf, (void*)m_WebRequest.GetResponseText(), itr->m_FileSize);
// 						itr->m_Delete = true;
// 						drgPrintOut("Downloaded: %s (%u)\r\n", itr->m_AssetName, itr->m_FileSize);
// 						drgFileCatalog::GetFileCatalog()->AddAsset(itr->m_AssetName, itr->m_MemBuf, itr->m_FileSize);
// 						itr->m_FilePtr = 0;
// 						itr->m_State = DRG_FILE_ASYNC_OPENED;
// 						if (itr->m_Callback != NULL)
// 							itr->m_Callback(itr->m_CallBackData);
// 						if (itr->m_State == DRG_FILE_ASYNC_OPENED)
// 							itr->m_State = DRG_FILE_ASYNC_READY;
// 					}
// 					else
// 					{
// 						drgPrintOut("Failed to download asset: %s\r\n", strNewName);
// 						itr->m_State = DRG_FILE_ASYNC_OPENING_CACHE;
// 					}
// 				}
// 				AddToQueue(itr, true);
// 			}
// 			else if (itr->m_State == DRG_FILE_ASYNC_CLOSEING)
// 			{
// 				itr->m_AssetMutex.Lock();
// 				if (itr->m_Delete && (itr->m_MemBuf != NULL))
// 					drgMemFree(itr->m_MemBuf);
// 				itr->m_MemBuf = NULL;
// 				itr->m_FileSize = 0;
// 				itr->m_FilePtr = 0;
// 				itr->m_Delete = false;
// 				itr->m_State = DRG_FILE_ASYNC_CLOSEED;
// 				if (itr->m_State == DRG_FILE_ASYNC_CLOSEED)
// 					itr->m_State = DRG_FILE_ASYNC_DONE;
// 				itr->m_InQueue = false;
// 				itr->m_AssetMutex.Unlock();
// 			}
// 			else
// 			{
// 				itr->m_AssetMutex.Lock();
// 				itr->m_State = DRG_FILE_ASYNC_DONE;
// 				itr->m_InQueue = false;
// 				itr->m_AssetMutex.Unlock();
// 				//drgPrintWarning("Async File State Wrong!\r\n");
// 				//AddToQueue(itr);
// 			}
// 		}
// 		else
// 		{
// 			drgThread::SleepCurrent(100);
// 		}
// 	}

// 	drgPrintOut("End of drgFileManager thread\r\n");
// 	m_Initialized = false;
// 	return 0;
// }

// void drgFileManager::CatalogLoadCB(void* data)
// {
// 	if (m_WebCatalogFile.GetFileSize()>0)
// 		m_WebCatalog.Init(m_WebCatalogFile.GetFileMem(), m_WebCatalogFile.GetFileSize());
// 	m_WebCatalogFile.Close();
// }

void drgFileManager::SetResourceFolder(const char* folder)
{
	char buffer[ DRG_MAX_PATH ];
	drgString::PrintFormated( buffer, "Setting resource folder to: %s\n", folder );
	drgDebug::Print( buffer );

	drgString::Copy(m_ResourceFolderBase, folder);
	// if (m_AssetServerBase[0] != '\0')
	// 	m_WebCatalogFile.OpenAsync(DRG_FILE_CATALOG_FILE, CatalogLoadCB, NULL, true);

	drgString::Copy(m_ResourceFolder, m_ResourceFolderBase);
	drgString::Concatenate(m_ResourceFolder, drgEngine::GetAppName());
	drgString::Concatenate(m_ResourceFolder, "/");
}

const char* drgFileManager::GetResourceFolder()
{
	return m_ResourceFolder;
}

void drgFileManager::SetCacheFolder(const char* folder)
{
	drgString::Copy(m_CacheFolderBase, folder);
}

const char* drgFileManager::GetCacheFolder()
{
	drgString::Copy(m_CacheFolder, m_CacheFolderBase);
	drgString::Concatenate(m_CacheFolder, drgEngine::GetAppName());
	drgString::Concatenate(m_CacheFolder, "/");
	return m_CacheFolder;
}

void drgFileManager::SetAssetServer(const char* folder)
{
	drgString::Copy(m_AssetServerBase, folder);
	// if (m_AssetServerBase[0] != '\0')
	// 	m_WebCatalogFile.OpenAsync(DRG_FILE_CATALOG_FILE, CatalogLoadCB, NULL, true);
}

const char* drgFileManager::GetAssetServer()
{
	if (m_AssetServerBase[0] == '\0')
		return m_AssetServerBase;
	drgString::Copy(m_AssetServer, m_AssetServerBase);
	drgString::Concatenate(m_AssetServer, drgEngine::GetAppName());
	drgString::Concatenate(m_AssetServer, "/");
	return m_AssetServer;
}

void drgFileManager::MakeResourceName(char* outname, const char* filename)
{
	drgString::Copy(outname, drgFile::GetResourceFolder());
	drgString::Concatenate(outname, filename);
}

//! Removes the resources folder filepath from the given absolute filepath.  CASE SENSITIVE
void drgFileManager::UnmakeResourceName(char* outname, const char* absoluteFilepath, int maxLen)
{
	const char* res_folder = drgFile::GetResourceFolder();
	unsigned int str_len = drgString::Length(drgFile::GetResourceFolder());
	if (!drgString::CompareNoCase(res_folder, absoluteFilepath, str_len))
		drgString::Copy(outname, &absoluteFilepath[str_len], maxLen);
	else
		drgString::Copy(outname, absoluteFilepath, maxLen);
}





#ifdef	__cplusplus
extern	"C" {
#endif

void* drgFileOpenExt(const char* filename)
{
	MEM_INFO_SET_NEW;
	drgAssetReader* reader = new drgAssetReader;
	reader->OpenFile(filename, true);
	if (!reader->IsGood())
	{
		delete reader;
		reader = NULL;
	}
	return reader;
}

void drgFileCloseExt(void* file)
{
	drgAssetReader* reader = (drgAssetReader*)file;
	reader->Close();
	delete reader;
}

int drgFileExistsExt(const char* filename)
{
#if 0
	FILE* file = fopen(filename, "rb");
	if (file)
	{
		fclose(file);
		return 1;
	}
	return 0;
#else
	return 1;
#endif
}

int drgFileReadExt(void* buff, unsigned int size, void* file)
{
	drgAssetReader* reader = (drgAssetReader*)file;
	return reader->Read(buff, size);
}

#ifdef	__cplusplus
};
#endif

