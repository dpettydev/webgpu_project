#ifndef __DRG_FILE_H__
#define __DRG_FILE_H__

#include "system/global.h"
#include "util/stringv.h"

enum DRG_FILE_ENTRY_TYPE
{
	DRG_FILE_ENTRY_TYPE_FILE,
	DRG_FILE_ENTRY_TYPE_FOLDER,
};

struct DRG_FILE_FOLDER_LISTNODE
{
	char name[DRG_MAX_PATH];
	DRG_FILE_ENTRY_TYPE type;
};

class drgFile
{
public:
	enum DRG_FILE_MODE
	{
		DRG_MODE_READ_BIN=0,
		DRG_MODE_READ_TEX,
		DRG_MODE_FORCE_WRITE_BIN,
		DRG_MODE_WRITE_BIN,
		DRG_MODE_FORCE_WRITE_TEX,
		DRG_MODE_WRITE_TEX,
		DRG_MODE_FORCE_APPEND_BIN,
		DRG_MODE_APPEND_BIN,
		DRG_MODE_FORCE_APPEND_TEX,
		DRG_MODE_APPEND_TEX,
		DRG_MODE_FORCE_RD_WR_BIN,
		DRG_MODE_RD_WR_BIN,
		DRG_MODE_FORCE_RD_WR_TEX,
		DRG_MODE_RD_WR_BIN_NEW,
		DRG_MODE_RD_WR_TEX
	};

	enum DRG_FILE_SEEK
	{
		DRG_SEEK_BEGIN		= 1,
		DRG_SEEK_CUR		= 2,
		DRG_SEEK_END		= 3
	};

	enum DRG_FOLDER_FLAGS
	{
		DRG_NO_FOLDERS		= (1<<0),
		DRG_NO_FILES		= (1<<1)
	};

	enum DRG_DATA_ARRAY_TYPE
	{
		DRG_DATA_ARRAY_DEFAULT=0,
		DRG_DATA_ARRAY_CPP,
		DRG_DATA_ARRAY_EMSCRIPT,
		DRG_DATA_ARRAY_MAX
	};

	drgFile();
	drgFile( const char* filename, DRG_FILE_MODE mode=DRG_MODE_READ_BIN );
	~drgFile();

	//! Opens a file
	//! \param filename The path to the file to open
	//! \param mode Permissions to open the file with
	//! \return True if the file was opened successfully
	bool Open( const char* filename, DRG_FILE_MODE mode=DRG_MODE_READ_BIN );

	//! Closes the file
	void Close();

	//! checks the file
	bool IsGood();
	bool IsEndOfFile();

	//! Reads up to size number of characters into the buffer out
	//! \param out The buffer to write the characters to
	//! \param size The size of the buffer
	unsigned int Read( void* out, unsigned int size );
	unsigned int ReadLine( string16* out, char terminator='\n' );

	//! Writes a buffer to the file
	//! \param out The buffer used to write to the file
	//! \param size The size of the buffer
	unsigned int Write( void* out, unsigned int size );
	unsigned int Write( string16* out );
	unsigned int WriteStr( const char* out );

	unsigned int PrintFormated (const char* fmt, ... );

	unsigned int WriteEndian8( void* out );
	unsigned int WriteEndian16( void* out );
	unsigned int WriteEndian32( void* out );
	unsigned int WriteEndian64( void* out );
	unsigned int WriteEndianPT2( void* out );
	unsigned int WriteEndianPT3( void* out );
	unsigned int WriteEndianPT4( void* out );
	unsigned int WriteEndianMT33( void* out );
	unsigned int WriteEndianMT34( void* out );
	unsigned int WriteEndianMT44( void* out );
	unsigned int WriteEndianBBox( void* out );

	inline static void SetEndianWrite( bool bigendian )
	{
		m_BigendianWrite=bigendian;
	}

	inline static bool GetEndianWrite()
	{
		return m_BigendianWrite;
	}

	inline static bool GetEndianRead()
	{
		return m_BigendianRead;
	}

	int ReadChar();
	unsigned int WriteChar(int ch);

	//! Seeks to a specified position in the file
	//! \param offset Number of positions to advance in the file from the current position
	//! \param from Where to start seeking from
	int Seek( int offset, DRG_FILE_SEEK from=DRG_SEEK_BEGIN );

	//! Seeks starting from 'from' to 'offset' number of positions, dumping the data into the buffer 'out'
	//! \param offset Number of positions to seek from the start point
	//! \param from The starting position in the file to seek from
	//! \param out Buffer to write data to
	//! \param size Size of the out buffer
	unsigned int SeekRead( int offset, DRG_FILE_SEEK from, void* out, unsigned int size );

	//! Gets the current position in the file
	//! \return The current position in the file
	unsigned int GetFilePos();

	//! Gets the file size
	//! \return The total file size
	unsigned int GetFileSize();

	//! Gets a string from the file stream
	//! \param out Pointer to a buffer where to store data.
	//! \param size Number of bytes to be read.
	char * GetString(char *out, int size);

	//! Writes the specified number of fill characters to the file
	//! \param size The number of fill characters to write
	//! \param fill The character to use as the fill character
	unsigned int WriteAlignment( unsigned int size, char fill=0 );

	//! Gets the working folder path for resources
	//! \return The working folder path for resources
	static const char* GetResourceFolder();
	static void GetHomePathFolder(char* outfolder);

	static int GetFolderNumFiles( const char* folder, const char* ext, int flags=0 , bool fullpath=true );
	static int GetFolderListFiles( const char* folder, const char* ext, DRG_FILE_FOLDER_LISTNODE* list, int maxnum, int flags=0, bool folderIsFullpath=true, bool outputIsFullpath=false );
	static string8* GetFileInFolderByIndex( const char* folder, const char* extensionFilter, int fileIndex, int flags=0 , bool fullpath=true );
	static bool FileExists( const char* filepath );
	static bool DirectoryExists( const char* directorypath );
	static void FixPath( char* folder );
	static void* MallocFile( const char* filename, unsigned int* size=NULL );
	static void* CopyFileData( const char* filename, void* mem, unsigned int max_size, unsigned int* size=NULL );
	static unsigned int GetFileDateModify( const char* file );

	static int MakeDir(const char *dirname);
	static void MakeDirTree(const char *dirname, int start);
	static int Rename(const char *oldname, const char *newname );
	static bool Copy(const char* src, const char* dst);
	static bool Delete(const char* file);

	static void CreateDataArray(char* filename, char* outfile, char* arrayname, DRG_DATA_ARRAY_TYPE type=DRG_DATA_ARRAY_DEFAULT);

	void*				m_File; //!< File pointer

protected:
	static bool			m_BigendianWrite;
	static bool			m_BigendianRead;

	unsigned int		m_FileSize; //!< File size
	unsigned int		m_FileFlag;
};



#endif // __DRG_FILE_H__

