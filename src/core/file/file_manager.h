#ifndef __DRG_FILE_OPERATION_MANAGER_H__
#define __DRG_FILE_OPERATION_MANAGER_H__

//#include "system/thread.h"
#include "core/file/file_async.h"
#include "system/global.h"

class drgMutex;

class drgFileManager //: public drgThreadRun
{
public:	
	drgFileManager();
	~drgFileManager();

	//unsigned int ThreadRun(drgThread* thread);

	static void Init();
	static void CleanUp();
	static void Update();

	//! Removes the resources folder filepath from the given absolute filepath.  CASE SENSITIVE
	static void UnmakeResourceName(char* outname, const char* absoluteFilepath, int maxLen);
	static void MakeResourceName(char* outname, const char* filename);

	//! Sets the working folder path for resources
	//! \param folder The folder path
	static void SetResourceFolder(const char* folder);
	//! Gets the working folder path for resources
	static const char* GetResourceFolder();

	static void SetCacheFolder(const char* folder);
	static const char* GetCacheFolder();

	static void SetAssetServer(const char* uri);
	static const char* GetAssetServer();


	//*** PROTECTED ***
protected:
	drgFileAsync*			m_RootNodeIdle;
	drgFileAsync*			m_RootNodeActive;
	// drgRequest				m_WebRequest;
	volatile bool			m_IsRunning;

	static drgFileManager*	m_Instance;
	//static drgThread*		m_AssetThread;
	static drgMutex			m_FileManagerMutex;
	static bool				m_Initialized;
	//static drgFileCatalog	m_WebCatalog;
	//static drgFileAsync		m_WebCatalogFile;

	static void AddToQueue(drgFileAsync* file, bool needs_lock);
	static void RemoveFromQueue(drgFileAsync* file);

	static void CatalogLoadCB(void* data);

	static char			m_CacheFolder[DRG_MAX_PATH];
	static char			m_CacheFolderBase[DRG_MAX_PATH];
	static char			m_ResourceFolder[DRG_MAX_PATH];
	static char			m_ResourceFolderBase[DRG_MAX_PATH];
	static char			m_AssetServerBase[512];
	static char			m_AssetServer[512];

	friend class drgFileAsync;
};

#endif //__DRG_FILE_OPERATION_MANAGER_H__
