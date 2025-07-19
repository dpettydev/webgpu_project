#if defined(_WINDOWS)

#include "system/extern_inc.h"
#include "system/file.h"
#include "system/engine.h"
#include "util/stringn.h"
#include "util/math_def.h"
#include "util/filename.h"
#include "core/file/file_manager.h"


bool drgFile::m_BigendianWrite = false;
bool drgFile::m_BigendianRead = false;


void drgFile::GetHomePathFolder(char* outfolder)
{
#if 0
	char* pPath = getenv ("ProgramFiles(x86)");  	// For x64 Windows
	if (pPath == NULL)
	{
		pPath = getenv ("ProgramFiles");	// For x86 Windows
		if (pPath == NULL)
			pPath = "C:\\Program Files"; // We couldn't find a good path. Make an assumption
	}
	drgString::PrintFormated(outfolder, "%s%s", pPath, &folder[11]);
#elif 0
	char pPath[DRG_MAX_PATH] = "C:\\AssetServer\\Projects";
//	HRESULT hr = SHGetFolderPath( g_hCurDlg, CSIDL_APPDATA|CSIDL_FLAG_CREATE, NULL, 0, pPath );
//	if(hr<0)
//		drgString::Copy(pPath, "C:\\AssetServer\\Projects");
	if(!SHGetSpecialFolderPathA( NULL, pPath, CSIDL_APPDATA|CSIDL_FLAG_CREATE, true ))
		drgString::Copy(pPath, "C:\\AssetServer\\Projects");
	drgString::PrintFormated(outfolder, "%s%s", pPath, &folder[11]);
#else
	drgString::Copy(outfolder, "C:");
#endif
}


drgFile::drgFile()
{
	m_File=NULL;
	m_FileSize=0;
}

drgFile::drgFile( const char* filename, DRG_FILE_MODE mode )
{
	m_File=NULL;
	Open( filename, mode );
	m_FileSize=0;
}

drgFile::~drgFile()
{
	Close();
}

const char* drgFile::GetResourceFolder()
{
	return drgFileManager::GetResourceFolder();
}

bool drgFile::Open( const char* filename, DRG_FILE_MODE mode )
{
	if(m_File != NULL )
		Close();
	
	char cmode[5]="rb\0\0";

	switch(mode)
	{
	case DRG_MODE_READ_BIN:
		cmode[0]='r';
		cmode[1]='b';
		break;
	case DRG_MODE_READ_TEX:
		cmode[0]='r';
		cmode[1]='t';
		break;
	case DRG_MODE_FORCE_WRITE_BIN:
		cmode[0]='w';
		cmode[1]='b';
		SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
		break;
	case DRG_MODE_WRITE_BIN:
		cmode[0]='w';
		cmode[1]='b';
		break;
	case DRG_MODE_FORCE_WRITE_TEX:
		cmode[0]='w';
		cmode[1]='t';
		SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
		break;
	case DRG_MODE_WRITE_TEX:
		cmode[0]='w';
		cmode[1]='t';
		break;
	case DRG_MODE_FORCE_APPEND_BIN:
		cmode[0]='a';
		cmode[1]='b';
		SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
		break;
	case DRG_MODE_APPEND_BIN:
		cmode[0]='a';
		cmode[1]='b';
		break;
	case DRG_MODE_FORCE_APPEND_TEX:
		cmode[0]='a';
		cmode[1]='t';
		SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
		break;
	case DRG_MODE_APPEND_TEX:
		cmode[0]='a';
		cmode[1]='t';
		break;
	case DRG_MODE_FORCE_RD_WR_BIN:
		cmode[0]='r';
		cmode[1]='+';
		cmode[2]='b';
		SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
		break;
	case DRG_MODE_RD_WR_BIN:
		cmode[0]='r';
		cmode[1]='+';
		cmode[2]='b';
		break;
	case DRG_MODE_FORCE_RD_WR_TEX:
		cmode[0]='r';
		cmode[1]='+';
		cmode[2]='t';
		SetFileAttributesA(filename, FILE_ATTRIBUTE_NORMAL);
		break;
	case DRG_MODE_RD_WR_BIN_NEW:
		cmode[0]='w';
		cmode[1]='+';
		cmode[2]='b';
		break;
	case DRG_MODE_RD_WR_TEX:
		cmode[0]='r';
		cmode[1]='+';
		cmode[2]='t';
		break;
	}
	m_File=NULL;
	m_File = (void*)fopen(filename, cmode);
//	fopen_s((FILE**)&m_File, filename, cmode);
	if(m_File)
	{
		//drgPrintOut("Open file size:%i (%s)\n", GetFileSize(), filename);
		return true;	
	}

	drgPrintOut("Could not open file (%s)\n%s\n",filename,strerror( errno ));
	return false;
}

void drgFile::Close()
{
	if(m_File)
	{
		fflush((FILE*)m_File);
		fclose((FILE*)m_File);
		m_File=NULL;
	}
	m_FileSize=0;
}

bool drgFile::IsGood()
{
	return !!m_File;
}

bool drgFile::IsEndOfFile()
{
	return !!feof( (FILE*)m_File );
}

unsigned int drgFile::Read( void* out, unsigned int size )
{
	return (unsigned int)fread( out, 1, size, (FILE*)m_File );
}

unsigned int drgFile::ReadLine( string16* out, char terminator )
{
	out->clear();
	unsigned int readnm = 0;
	unsigned int len = 0;
	char getch;
	do
	{
		readnm = (unsigned int)fread( &getch, 1, 1, (FILE*)m_File );
		if(readnm==0)
			break;
		(*out) += getch;
		len++;
	} while(getch!=terminator);
	return len;
}

unsigned int drgFile::Write( void* out, unsigned int size )
{
	return (unsigned int)fwrite( out, 1, size, (FILE*)m_File );
}

unsigned int drgFile::Write( string16* out )
{
	string8 newout(*out);
	return (unsigned int)fwrite( newout.c_str(), 1, out->length(), (FILE*)m_File );
}

unsigned int drgFile::WriteStr( const char* out )
{
	return (unsigned int)fwrite( out, 1, drgString::Length(out), (FILE*)m_File );
}


unsigned int drgFile::PrintFormated(const char* fmt, ... )
{
	va_list argp;
	va_start(argp, fmt);
	int ret=vfprintf((FILE*)m_File, fmt, argp);
	va_end(argp);

	return ret;
}


unsigned int drgFile::WriteEndian16( void* out )
{
	drgUInt16 writeval=*((drgUInt16*)out);
	char* tempptr=(char*)&writeval;
	if(m_BigendianWrite)
	{
		char tempval=tempptr[0];
		tempptr[0]=tempptr[1];
		tempptr[1]=tempval;
	}
	return Write(&writeval,2);
}

unsigned int drgFile::WriteEndian32( void* out )
{
	drgUInt32 writeval=*((drgUInt32*)out);
	char* tempptr=(char*)&writeval;
	if(m_BigendianWrite)
	{
		char tempval=tempptr[0];
		tempptr[0]=tempptr[3];
		tempptr[3]=tempval;
		tempval=tempptr[1];
		tempptr[1]=tempptr[2];
		tempptr[2]=tempval;
	}
	return Write(&writeval,4);
}

unsigned int drgFile::WriteEndian64( void* out )
{
	drgUInt64 writeval=*((drgUInt64*)out);
	char* tempptr=(char*)&writeval;
	if(m_BigendianWrite)
	{
		char tempval=tempptr[0];
		tempptr[0]=tempptr[7];
		tempptr[7]=tempval;
		tempval=tempptr[1];
		tempptr[1]=tempptr[6];
		tempptr[6]=tempval;
		tempval=tempptr[2];
		tempptr[2]=tempptr[5];
		tempptr[5]=tempval;
		tempval=tempptr[3];
		tempptr[3]=tempptr[4];
		tempptr[4]=tempval;
	}
	return Write(&writeval,8);
}

unsigned int drgFile::WriteEndianPT2( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianP2(out);
	return Write(out,sizeof(float)*2);
}

unsigned int drgFile::WriteEndianPT3( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianP3(out);
	return Write(out,sizeof(float)*3);
}

unsigned int drgFile::WriteEndianPT4( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianP4(out);
	return Write(out,sizeof(float)*4);
}

unsigned int drgFile::WriteEndianMT33( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianM33(out);
	return Write(out,sizeof(float)*3*3);
}

unsigned int drgFile::WriteEndianMT34( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianM34(out);
	return Write(out,sizeof(float)*3*4);
}

unsigned int drgFile::WriteEndianMT44( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianM44(out);
	return Write(out,sizeof(float)*4*4);
}

unsigned int drgFile::WriteEndianBBox( void* out )
{
	if(m_BigendianWrite)
		drgFlipEndianBBox(out);
	return Write(out,sizeof(float)*6);
}

unsigned int drgFile::WriteChar(int ch)
{
	return (unsigned int)fputc(ch, (FILE*)m_File);
}

int drgFile::ReadChar()
{
	return fgetc((FILE*)m_File);
}

int drgFile::Seek( int offset, DRG_FILE_SEEK from )
{
	int ret = 0;
	switch(from)
	{
	case DRG_SEEK_END:
		ret = fseek( (FILE*)m_File, offset, SEEK_END );
		break;
	case DRG_SEEK_BEGIN:
		ret = fseek( (FILE*)m_File, offset, SEEK_SET );
		break;
	case DRG_SEEK_CUR:
		ret = fseek( (FILE*)m_File, offset, SEEK_CUR );
		break;
	}
	return ret;
}

unsigned int drgFile::SeekRead( int offset, DRG_FILE_SEEK from, void* out, unsigned int size )
{
	Seek (offset, from);
	return Read(out, size);
}

unsigned int drgFile::GetFilePos()
{
	return ftell( (FILE*)m_File );
}

unsigned int drgFile::GetFileSize()
{
	if(m_File && !m_FileSize)
	{
		unsigned int curPos=GetFilePos();
		Seek( 0, DRG_SEEK_END );
		m_FileSize=GetFilePos();
		Seek( curPos, DRG_SEEK_BEGIN );
	}
	return m_FileSize;
}

char * drgFile::GetString(char * out, int size)
{
	return fgets(out, size, (FILE*)m_File);
}

unsigned int drgFile::WriteAlignment( unsigned int size, char fill )
{
	while(GetFilePos()&(size-1)) Write(&fill,1);
	return GetFilePos();
}

int drgFile::GetFolderNumFiles( const char* folder, const char* ext, int flags, bool fullpath )
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	int numfiles = 0;

	char tmpfolder[DRG_MAX_PATH];

	if (fullpath)
	{
		drgString::Copy( tmpfolder, folder );
	}
	else
	{
		drgString::Copy(tmpfolder, drgFileManager::GetResourceFolder());
		drgString::Concatenate(tmpfolder, folder);
	}
	drgString::Concatenate( tmpfolder, "\\*." );

	if( ext )
	{
		drgString::Concatenate( tmpfolder, ext );
	}
	else
	{
		drgString::Concatenate( tmpfolder, "*" );
	}

	hFind = FindFirstFileA(tmpfolder, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if( (flags&DRG_NO_FOLDERS) && (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
				continue;
			if( (flags&DRG_NO_FILES) && !(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
				continue;
			if( !drgString::Compare( FindFileData.cFileName, "." ) )
				continue;
			if( !drgString::Compare( FindFileData.cFileName, ".." ) )
				continue;
			numfiles++;
		} while( FindNextFileA(hFind, &FindFileData) );
		FindClose(hFind);
	}
	return numfiles;
}

int drgFile::GetFolderListFiles( const char* folder, const char* ext, DRG_FILE_FOLDER_LISTNODE* list, int maxnum, int flags, bool folderIsFullpath, bool outputIsFullpath )
{
	WIN32_FIND_DATAA FindFileData;
	HANDLE hFind;
	int numfiles = 0;

	char tmpfolder[DRG_MAX_PATH];

	if (folderIsFullpath)
	{
		drgString::Copy( tmpfolder, folder );
	}
	else
	{
		drgString::Copy(tmpfolder, drgFileManager::GetResourceFolder());
		drgString::Concatenate(tmpfolder, folder);
	}
	drgString::Concatenate( tmpfolder, "\\*." );
	if( ext )
	{
		drgString::Concatenate( tmpfolder, ext );
	}
	else
	{
		drgString::Concatenate( tmpfolder, "*" );
	}

	hFind = FindFirstFileA(tmpfolder, &FindFileData);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if( (flags&DRG_NO_FOLDERS) && (FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
				continue;
			if( (flags&DRG_NO_FILES) && !(FindFileData.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) )
				continue;
			if( !drgString::Compare( FindFileData.cFileName, "." ) )
				continue;
			if( !drgString::Compare( FindFileData.cFileName, ".." ) )
				continue;

			//
			// Copy the file path, and list the type.
			//

			drgString::Copy( list[numfiles].name, FindFileData.cFileName );
			if(outputIsFullpath)
			{
				//Prepend the folder
				char tmp[DRG_MAX_PATH];
				drgString::Copy(tmp, folder);
				drgString::Concatenate(tmp, list[numfiles].name);
				drgString::Copy( list[numfiles].name, tmp ); 
			}

			if( FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
				list[ numfiles ].type = DRG_FILE_ENTRY_TYPE_FOLDER;

			if( !(FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				list[ numfiles ].type = DRG_FILE_ENTRY_TYPE_FILE;

			numfiles++;
		} while( (numfiles<maxnum) && FindNextFileA(hFind, &FindFileData) );
		FindClose(hFind);
	}
	return numfiles;
}

string8* drgFile::GetFileInFolderByIndex( const char* folder, const char* extensionFilter, int fileIndex, int flags, bool fullpath )
{
	const char* filter = extensionFilter == NULL ? "*" : extensionFilter;

	int numFiles = drgFile::GetFolderNumFiles( folder, extensionFilter, flags, true );
	if( numFiles == 0 )
		return NULL;

	DRG_FILE_FOLDER_LISTNODE* fileList = (DRG_FILE_FOLDER_LISTNODE*)drgMemAlloc( sizeof(DRG_FILE_FOLDER_LISTNODE) * (numFiles) );
	drgFile::GetFolderListFiles( folder, filter, fileList, numFiles, flags, true, true ); 

	string8* resultFilepath = new string8(fileList[ fileIndex ].name);

	SAFE_FREE( fileList );
	return resultFilepath;
}

bool drgFile::FileExists( const char* filepath )
{
	DWORD attr = GetFileAttributesA(filepath);
	return !((attr == INVALID_FILE_ATTRIBUTES) || (attr & FILE_ATTRIBUTE_DIRECTORY));
}

bool drgFile::DirectoryExists( const char* dirpath )
{
	DWORD attr = GetFileAttributesA(dirpath);
	return !((attr == INVALID_FILE_ATTRIBUTES) || !(attr & FILE_ATTRIBUTE_DIRECTORY));
}

void drgFile::FixPath( char* folder )
{
	char* charpos1=folder;
	char* charpos2=folder;
	while(charpos1[0]!='\0')
	{
		if( charpos1[0]=='/' )
			charpos1[0]='\\';
		charpos1++;
	}

	charpos1=folder;
	charpos2=folder;
	while(charpos1[0]!='\0')
	{
		if( charpos1[1] != '\0' )
		{
			if( charpos1[0]=='\\' && charpos1[1]=='.' && (charpos1[2]=='\\' || charpos1[2]=='\0') )
			{
				charpos1+=2;
				continue;
			}

			if( charpos1[0]=='\\' && charpos1[1]=='.' && charpos1[2]=='.' && (charpos1[3]=='\\' || charpos1[3]=='\0') )
			{
				charpos1+=3;
				while( (charpos2!=folder) && ((--charpos2)[0]!='\\') );
				continue;
			}
		}

		charpos2[0]=charpos1[0];
		charpos1++;
		charpos2++;
	}
	charpos2[0]=charpos1[0];
	charpos2=folder;
}

void* drgFile::MallocFile( const char* filename, unsigned int* size )
{
	drgFile memfile;
	unsigned int filesize;
	void* ret=NULL;
	if(!memfile.Open( filename, DRG_MODE_READ_BIN ))
		return ret;
	filesize=memfile.GetFileSize();
	if(size)
		(*size)=filesize;
	if(filesize)
	{
		ret=drgMemAlloc(filesize);
		memfile.Read(ret,filesize);
	}
	memfile.Close();
	return ret;
}

void* drgFile::CopyFileData( const char* filename, void* mem, unsigned int max_size, unsigned int* size )
{
	drgFile memfile;
	unsigned int filesize;
	void* ret=NULL;
	if(!memfile.Open( filename, DRG_MODE_READ_BIN ))
		return ret;
	filesize=memfile.GetFileSize();
	if(size)
		(*size)=filesize;
	if(filesize && (filesize<=max_size))
	{
		ret=mem;
		memfile.Read(ret,filesize);
	}
	memfile.Close();
	return ret;
}

bool drgFile::Copy( const char* src, const char* dst)
{
	drgFile srcFile;
	drgFile dstFile;

	unsigned int filesize;
	if(!srcFile.Open(src, DRG_MODE_READ_BIN))
		return false;
	filesize = srcFile.GetFileSize();
	
	void* buffer;
	if(filesize)
	{
		buffer = drgMemAlloc(filesize);
		srcFile.Read(buffer, filesize);
		srcFile.Close();

	}
	else
		return false;

	if(!dstFile.Open(dst, DRG_MODE_WRITE_BIN))
		return false;

	unsigned int ret = dstFile.Write(buffer, filesize);
	drgMemFree(buffer);
	if(ret != filesize)
		return false;

	dstFile.Close();

	return true;
}

bool drgFile::Delete(const char* file)
{
	return !!DeleteFileA( file );
}

unsigned int drgFile::GetFileDateModify( const char* file )
{
//	struct _stat buf;
//	int result = _stat( file, &buf );
//	if( result != 0 )
//		return 0;
	return (unsigned int)0;//buf.st_mtime;
}

int drgFile::MakeDir(const char *dirname)
{
	int ret=_mkdir(dirname);

	if (ret==0)
		return ret;


	if (ret==-1 && errno==EEXIST)
		return 1;

	return ret;
}

void drgFile::MakeDirTree(const char *dirname, int start)
{
	char dirname_tmp[DRG_MAX_PATH];
	drgString::Copy(dirname_tmp, dirname);
	for (char curval = dirname_tmp[start]; curval != '\0'; curval = dirname_tmp[start])
	{
		if(curval == '\\' || curval == '/')
		{
			dirname_tmp[start] = '\0';
			drgFile::MakeDir(dirname_tmp);
		}
		dirname_tmp[start] = curval;
		start++;
	}
}

int drgFile::Rename(const char *oldname, const char *newname )
{
	return rename(oldname, newname);
}

void drgFile::CreateDataArray(char* filename, char* outfile, char* arrayname, DRG_DATA_ARRAY_TYPE type)
{
	unsigned int filelen = 0;
	unsigned char* data = (unsigned char*)drgFile::MallocFile(filename, &filelen);
	drgFilename fileinfo(filename);
	if(filelen>0)
	{
		drgFile codefile;
		char temst[256];
		unsigned int pos = 1;
		unsigned int writelen = 0;
		codefile.Open(outfile, drgFile::DRG_MODE_WRITE_TEX);
		if(codefile.IsGood())
		{
			if(type==DRG_DATA_ARRAY_EMSCRIPT)
			{
				writelen = sprintf(temst, "FSData.%s = [%u", arrayname, data[0]);
				codefile.Write(temst, writelen);
				while(pos<filelen)
				{
					writelen = sprintf(temst, ",%u", data[pos++]);
					codefile.Write(temst, writelen);
				}
				writelen = sprintf(temst, "];\r\n");
				codefile.Write(temst, writelen);
			}
			else // DRG_DATA_ARRAY_DEFAULT, DRG_DATA_ARRAY_CPP
			{
				writelen = sprintf(temst, "\r\nunsigned char %s[%u] = {%u", arrayname, filelen, data[0]);
				codefile.Write(temst, writelen);
				while(pos<filelen)
				{
					writelen = sprintf(temst, ",%u", data[pos++]);
					codefile.Write(temst, writelen);
				}
				writelen = sprintf(temst, "};\r\n");
				codefile.Write(temst, writelen);
			}
		}
		codefile.Close();
	}
	drgMemFree(data);
}




#endif

