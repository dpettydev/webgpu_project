#ifndef __VGT_FILE_ASYNC_H_
#define __VGT_FILE_ASYNC_H_

#include "system/file.h"
#include "core/file/asset_read.h"

enum DRG_FILE_ASYNC_STATE
{
	DRG_FILE_ASYNC_NOT_ASYNC,
	DRG_FILE_ASYNC_INIT,
	DRG_FILE_ASYNC_OPENING,
	DRG_FILE_ASYNC_OPENING_CACHE,
	DRG_FILE_ASYNC_OPENING_WEB,
	DRG_FILE_ASYNC_OPENED,
	DRG_FILE_ASYNC_IDLE_WEB,
	DRG_FILE_ASYNC_READY,
	DRG_FILE_ASYNC_CLOSEING,
	DRG_FILE_ASYNC_CLOSEED,
	DRG_FILE_ASYNC_DONE
};

class drgFileAsync : public drgAssetReader
{
public:	
	drgFileAsync(drgInt32 priority = 50);
	virtual ~drgFileAsync();

	bool OpenAsync(const char* filename, drgCallback callback, void* data, bool check_web = false);
	bool OpenAsync(const char* filename, bool check_web);
	virtual void Close();

	inline bool IsReady()
	{
		return (m_State == DRG_FILE_ASYNC_READY);
	}

	//*** PROTECTED ***
protected:
	static void HandleOpenComplete(drgFileAsync* fileAsync);

	static void DefaultFileCallback(void* data);

	DRG_FILE_ASYNC_STATE	m_State;
	drgInt32				m_Priority;
	bool					m_InQueue;
	drgCallback				m_Callback;
	void*					m_CallBackData;
	drgMutex				m_AssetMutex;
	drgFileAsync*			m_Next;

	friend class drgFileManager;
};

#endif //__VGT_FILE_ASYNC_H_
