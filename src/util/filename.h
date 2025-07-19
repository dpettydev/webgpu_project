#ifndef __DRG_FILENAME_H__
#define __DRG_FILENAME_H__

#include "system/global.h"
#include "util/stringn.h"

//!Class that contains information about and methods for manipulating paths and filenames.
class drgFilename
{
public:
    drgFilename();
    drgFilename(const char* pFullPath);
	~drgFilename();

	//!Function to set the directory
	/*!
		\param dir new directory string
	*/
	void SetDir(const char* dir)					{ drgString::Copy(m_Dir, dir); }
	//!Function to set the drive
	/*!
		\param drive new drive string
	*/
	void SetDrive(const char* drive)				{ drgString::Copy(m_Drive, drive); }
	//!Function to set the extension
	/*!
		\param ext new extension string
	*/
	void SetExt(const char* ext)					{ drgString::Copy(m_Ext, ext); }
	//!Function to set the filename
	/*!
		\param fname new filename string
	*/
	void SetFilename(const char* fname)				{ drgString::Copy(m_Fname, fname); }
	//!Function to set the platform subdirectory
	/*!
		\param subdir new subdirectory string
	*/
	void SetPlatformSubDir(const char* subdir)		{ drgString::Copy(m_SubDir, subdir); }

	//!Function to get the directory string
	/*!
		\returns the directory string
	*/
	const char* GetDir() const						{ return m_Dir; }
	//!Function to get the drive string
	/*!
		\returns the drive string
	*/
	const char* GetDrive() const					{ return m_Drive; }
	//!Function to get the extension string
	/*!
		\returns the extension string
	*/
	const char* GetExt() const						{ return m_Ext; }
	//!Function to get the filename string
	/*!
		\returns the filename string
	*/
	const char* GetFilename() const					{ return m_Fname; }
	//!Function to get the subdirectory string
	/*!
		\returns the subdirectory string
	*/
	const char* GetPlatformSubDir() const			{ return m_SubDir; }

    void GetFullPath(char *pFullPath) const;

	void GetPath(char *pOutPath) const;

    void Splitpath(const char* pStr);

	void SetCaseFilename( bool upper );
	void SetCaseExt( bool upper );
	void SetCaseDrive( bool upper );
	void SetCaseDir( bool upper );
	void SetCasePlatformSubDir( bool upper );

	static void RemoveDoubleDotsSlashes(char* path);

private:
    void Makepath(char* pStr, bool bIncludeFile=true) const;
	void SetCase( char* str, bool upper );

	//!Directory String
    char m_Dir[DRG_MAX_PATH];
	//!Drive String
    char m_Drive[10];
	//!Extenstion String
    char m_Ext[16];
	//!Filename string
    char m_Fname[64];
	//!Subdirector String
    char m_SubDir[DRG_MAX_PATH];
};

#endif // __DRG_FILENAME_H__


