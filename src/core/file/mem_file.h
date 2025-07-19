#ifndef __DRG_MEM_FILE_H_
#define __DRG_MEM_FILE_H_

#include "system/global.h"
#include "system/file.h"


///<summary> Memory-cached file, with deferred reads and writes. </summary>
class drgMemFile
{
public:		
	enum DRG_MEM_FILE_TYPE
	{
		DRG_MEM_FILE_TYPE_NONE = 0,
		DRG_MEM_FILE_TYPE_FILE,
		DRG_MEM_FILE_TYPE_BUFFER,
		DRG_MEM_FILE_TYPE_COUNT,
	};
	
	///<summary>Null constructor.  Open() must be called by the caller.</summary>
	drgMemFile();

	///<summary>Constructor for files.  Automatically calls Open().</summary>
	///<param name="fullyQualifiedFilename">a full filepath-filename</param>
	///<param name="writeOnlyMode">
	///whether any read operations will be performed.  
	///True if no read operations will be performed.  For optimization.
	///</param>
	drgMemFile( const char* fullyQualifiedFilename, bool writeOnlyMode = false );

	///<summary>Constructor for reading from / writing to a buffer.  Automatically calls Open().</summary>
	///<param name="buff">the buffer</param>
	///<param name="bufflen">The length of the buffer.</param>
	///<param name="freeBufferUponClose">whether to call free on the buffer when this drgMemFile is closed</param>
	drgMemFile( void* buffer, drgInt32 bufflen, bool freeBufferUponClose = false );

	///<summary>Destructor.  Automatically calls Close() if open</summary>
	~drgMemFile();	

	///<summary>
	///Opens a file for use with drgMemFile.  Must only be called if null constructor was used or drgMemFile.Close() was used.  
	///Moves the file pointer.
	///</summary>
	///<param name="fullyQualifiedFilename">a full filepath-filename.</param>
	///<param name="writeOnlyMode">Set to true if only write operations will be used.  The file length will reset to 0.  For optimization.</param>
	///<param name="from">Where the file pointer will be upon open.</param>
	///<returns>true if successful</returns>
	bool Open( const char* fullyQualifiedFilename, bool writeOnlyMode = false, drgFile::DRG_FILE_SEEK from = drgFile::DRG_SEEK_CUR );

	///<summary>
	///Opens a buffer for use with drgMemFile.  Must only be called if null constructor was used or drgMemFile.Close() was used.  
	///Moves the file pointer.
	///</summary>
	///<param name="buffer">The buffer to read from / write to.</param>
	///<param name="bufflen">The length of the buffer.</param>
	///<param name="freeBufferUponClose">whether to call free on the buffer when this drgMemFile is closed</param>
	///<returns>true if successful</returns>
	bool Open(void* buffer, drgUInt32 bufflen, bool freeBufferUponClose = false, drgFile::DRG_FILE_SEEK from = drgFile::DRG_SEEK_CUR );
	
	///<summary>
	///Synchronizes the "file" in memory with the file on disk, overwriting the "file" on memory.  Does not move the current file position.
	///ALL UNSYNCED WRITES MAY BE LOST.  Use SyncWritesToFile() to sync writes to disk.
	///</summary>
	///<returns>The number of bytes actualy read from disk, or zero if the file did not need to be updated or no data was read from disk. </returns>
	drgUInt32 UpdateFromLatestFile();

	///<returns></returns>
	bool IsGood();

	///<returns>true if the file position is at or past the end of the file.</returns>
	bool IsEndOfFile();	

	unsigned int GetFileSize();

	void* GetMemBuff();

	///<summary>Moves the current file position.</summary>
	///<param name="offset">the number of bytes past or before the 'from' to move the file pointer</param>
	///<param name="from">Beginning of the file, current file pointer, or end of the file</param>
	///<returns>true if successful</returns>
	bool Seek( drgInt32 offset, drgFile::DRG_FILE_SEEK from=drgFile::DRG_SEEK_BEGIN );	
	
	///<returns>the current file position, in bytes, relative to the beginning of the file.</returns>
	drgUInt32 GetCurrentFilePosition();	

	///<summary>Reads from the drgMemFile.  Moves the current file position.</summary>
	///<param name="out">the output buffer into which the read data is placed </param>
	///<param name="length">the number of bytes desired to be read.</param>
	///<returns>zero if the read failed, or the number of bytes actually read</returns>
	drgUInt32 Read( void* out, drgUInt32 length );

	///<summary></summary>
	drgUInt32 ReadEndian8( void* out );

	///<summary></summary>
	drgUInt32 ReadEndian16( void* out );

	///<summary></summary>
	drgUInt32 ReadEndian32( void* out );

	///<summary>
	///Writes to the drgMemFile.  Moves the current file position.  
	///Writes will not be synchronized with the file on disk until Close() or SyncWritesToFile() are called.
	///</summary>
	///<param name="input">the input buffer which gets written to the drgMemFile </param>
	///<param name="length">the number of bytes desired to be written</param>
	///<param name="force">whether the write will be forced </param>
	///<param name="truncateFile">whether to truncate the file data to the end of this write.</param>
	///<returns>zero if the write failed, or the number of bytes written</returns>
	drgUInt32 Write( void* input, drgUInt32 length, bool force = false, bool truncateFile = false );

	///<summary>
	///Writes to the drgMemFile.  Moves the current file position.  
	///Writes will not be synchronized with the file on disk until Close() or SyncWritesToFile() are called.
	///</summary>
	///<param name="input">the input string which gets written to the drgMemFile </param>
	///<param name="force">whether the write will be forced </param>
	///<param name="truncateFile">whether to truncate the file data to the end of this write.</param>
	///<returns>zero if the write failed, or the number of bytes written</returns>
	drgUInt32 WriteStr( char* input, bool force = false, bool truncateFile = false );	

	///<summary>
	///Appends to the end of the drgMemFile.  Moves the current file position.  
	///Writes will not be synchronized with the file on disk until Close() or SyncWritesToFile() are called.  The file size will be increased.
	///</summary>
	///<param name="input">the input buffer which gets written to the drgMemFile </param>
	///<param name="length">the number of bytes desired to be written</param>
	///<param name="force">whether the write will be forced </param>
	///<param name="truncateFile">whether to truncate the file data to the end of this write.</param>
	///<returns>zero if the append failed, or the number of bytes written</returns>
	drgUInt32 Append( void* input, drgUInt32 length, bool force = false, bool truncateFile = false );

	///<summary>Actually writes to the file system: flushes all previous writes to the file on disk.  Does not move the current file position</summary>
	///<returns>The number of bytes actualy written to disk, or zero if the write failed or the file is unchanged. </returns>
	drgUInt32 SyncWritesToFile();
		
	///<summary>
	///Closes the file, and synchronizes the writes to the file on disk, if necissary.  
	///Releases resources.  Does not move the current file position.
	///</summary>
	///<returns>true if successful</returns>
	bool Close(); 

	///<summary>Deletes all data from the file on disk.  Does not move the current file position</summary>
	///<returns>True if successful.</returns>
	bool Clear();

protected:
	static bool				m_BigendianRead;
	
	DRG_MEM_FILE_TYPE		m_Type;
	unsigned char*			m_MemBuf;
	drgUInt32				m_FilePtr;
	drgUInt32				m_FileSize;
	string8*				m_pFilename;
	bool					m_DeleteBuf;
	bool					m_IsOpen;
	bool					m_PendingUnsyncedWrites;
	bool					m_ForceWrite;
	bool					m_WriteOnlyMode; 

	void Initialize();

	drgUInt32 ResizeBuffer(drgUInt32 newLength);

	void SetFilename(const char* fullyQualifiedFilename);

	bool FreeMemBuff();
};


#endif //__DRG_MEM_FILE_H_
