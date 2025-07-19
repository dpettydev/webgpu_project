#ifndef __DRG_MEM_ASSET_H_
#define __DRG_MEM_ASSET_H_

#include "system/global.h"
#include "system/file.h"
#include "core/callbacks.h"


class drgAssetReader
{
public:
	drgAssetReader();
	virtual ~drgAssetReader();

	bool OpenFile(const char* filename, bool check_web = false);
	bool OpenBuff(void* buff, int bufflen);
	bool OpenBuff(int bufflen);
	virtual void Close();

	const char* GetName();
	unsigned int GetFileSize();
	unsigned int GetFilePos();
	unsigned char* GetFileMem();
	unsigned char* TakeOverFileMem();  // memory is not deleted after this reader is done

	bool IsGood();
	bool IsEndOfFile();
	void Seek(int offset, drgFile::DRG_FILE_SEEK from = drgFile::DRG_SEEK_BEGIN);
	unsigned int Read(void* out, unsigned int size);
	unsigned int ReadEndian8(void* out);
	unsigned int ReadEndian16(void* out);
	unsigned int ReadEndian32(void* out);
	string8 ReadLine(unsigned int* _length = NULL);

protected:
	char			m_AssetName[DRG_MAX_PATH];
	unsigned char*	m_MemBuf;
	unsigned int	m_FilePtr;
	unsigned int	m_FileSize;
	bool			m_Delete;
	drgMutex		m_AssetMutex;
};



#endif //__DRG_MEM_FILE_H_
