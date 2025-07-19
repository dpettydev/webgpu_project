#include "util/filename.h"
#include "util/math_def.h"

//!Constructor accepting the full path of a file.
/*!
	\param pFullPath full path of a file
*/
drgFilename::drgFilename(const char* pFullPath)
{
    m_SubDir[0] = '\0';

    // Break down the file name here
    Splitpath(pFullPath);
}

//!Constructor accepting the full path of a file.
/*!
	\param pFullPath full path of a file
*/
drgFilename::drgFilename()
{
    m_Dir[0] = '\0'; 
    m_Drive[0] = '\0';
    m_Ext[0] = '\0';
    m_Fname[0] = '\0';
    m_SubDir[0] = '\0'; 
    m_SubDir[0] = '\0';
}

//!Destructor
drgFilename::~drgFilename()
{
}

//!Function that generates the full path for a file
/*!
	\param pFullPath location to store the created path
*/
void drgFilename::GetFullPath(char* pFullPath) const
{
    Makepath(pFullPath);
}

//! Function that generates the path to the file
/*
 *
 *		@param[out]		pOutPath	Destination of path.
 *
*/
void drgFilename::GetPath(char *pOutPath) const
{
	Makepath(pOutPath, false);
}

static int FromBack(char c, const char* pStr, int hardLimit)
{
	int iLen = (int)drgString::Length(pStr);

    // Search for the character starting at the back of the string
    for(int j=iLen - 1; j >= hardLimit; j--) 
        if (pStr[j] == c) 
            return j;

    // Didn't find the value so return a sentinel value of -1
    return -1;
}

void drgFilename::RemoveDoubleDotsSlashes(char* path)
{
	char *pstr = path;
	while(*path != '\0')
	{
		if(*path == '\\')
			*path = '/';
		*pstr++ = *path++;
		if(path[-1] == '/')
		{
			// Skip all following slashes and backslashes
			while(*path == '/' || *path == '\\')
				path++;

			// Skip all double-dots
			while(*path == '.' && path[1] == '.')
				path += 2;
		}
	}
	*pstr = '\0';
	pstr = path;
	while(*path != '\0')
	{
		*pstr++ = *path++;
		if(path[-1] == '/')
		{
			// Skip all following slashes and backslashes
			while(*path == '/')
				path++;
		}
	}
	*pstr = '\0';
}


//!Helper function that parses a path into it's respective drive, directory, filename and extension.
/*!
	\param pStr string to parse
*/
void drgFilename::Splitpath(const char* pStr)
{
    // One last monkey wrench: people can pass in NULL arguments to indicate
    //   that that variable doesn't need to be filled in.

    // 0. Initialize the strings that aren't NULL pointers
    m_Drive[0] = '\0';
    m_Ext[0] = '\0';
    m_Dir[0] = '\0';
    m_Fname[0] = '\0';

    if (pStr == NULL)
        return;     // exit early if we're passed a NULL string to parse

    // 1. Initialize the bounds (can be shrunk by drive letter or extension)
    int startPos = 0;
    int endPos = (int)drgString::Length(pStr);
    // Assert: startPos points to position 0
    // Assert: endPos points to \0 at end of string
    if (endPos == 0)
        return;     // exit early if we're passed an empty string

    // 2. Is there a drive letter? Only check the second character, just 
    //    like WIN32.  Need to make sure the string is long enough first.
    if (endPos >= 2 && pStr[1] == ':') 
    { // Yes, there's a drive letter!
        m_Drive[0] = pStr[0];
        m_Drive[1] = ':';
        m_Drive[2] = '\0';
        startPos = 2;
    } // otherwise, startPos remains at the beginning of the string
    // Assert: startPos points to position 0, or position right after :
    // Assert: endPos points to \0 at end of string

    // 3. Search from right for directory separator and take the rightmost.
    //   (but don't search backwards past the drive letter! e.g. \:bla should
    //   not find the slash)
    int iPosDS = DRG_MAX(FromBack('\\', pStr, startPos), 
                     FromBack('/', pStr, startPos));
    // Assert: startPos points to pos 0, or pos right after "drive letter:"
    // Assert: endPos points to \0 at end of string

    // 4. Search from right for period. If period exists:
    //        If pos(period) < pos(directory separator), ext=empty
    //        else ext = pos(period)->end-of-string, endPos=pos(period)
    int iPosPeriod = FromBack('.', pStr, startPos);
    // Is there a period?
    if (iPosPeriod != -1)
    {
        // Does the rightmost period occur after the rightmost DS?
        if (iPosPeriod > iPosDS) // If DSpos=-1 (no DS), still works
        {
            drgString::Copy(m_Ext, pStr+iPosPeriod);
            endPos = DRG_MIN(endPos, iPosPeriod);
        } // otherwise (DS is to right of period), ext is empty
    }
    // Assert: startPos points to pos 0, or pos right after "drive letter:"
    // Assert: endPos points to \0 at end of string, or . in extension
    
    // 5. If dir. sep. exists, dir=startPos to DSpos, startPos = DSpos+1

    // Is there a directory separator?
    if (iPosDS != -1)
    {
        int howMany = iPosDS-startPos+1; // include the final char
        drgString::Copy(m_Dir, pStr+startPos, DRG_MAX(0, howMany));
        m_Dir[howMany] = '\0';
        // set the start position to the start of the Filename (slashpos+1)
        startPos = iPosDS+1;
		
		RemoveDoubleDotsSlashes(m_Dir);
    }
    // Assert: startPos points to pos 0, pos right after "drive letter:"
    //         (on non-Mac), or pos after directory separator
    // Assert: endPos points to \0 at end of string, or . in extension

    // 6. name = startPos->endPos-1 + \0
    // Copy the filename (startPos->endPos)
    int howMany = endPos-startPos; // don't include the final char
    drgString::Copy(m_Fname, pStr+startPos, DRG_MAX(0, howMany));
    m_Fname[howMany] = '\0';
}

//!Helper function that generates a full path string from the individual drive, directory, filename, and extension.
/*!
	\param pStr output string to build
*/
void drgFilename::Makepath(char* pStr, bool bIncludeFile) const
{
    if (pStr != NULL)
    {
        pStr[0] = '\0'; // Initialize the destination string

        // Copy the drive letter, if provided
        if (m_Drive[0] != '\0')
        {
            pStr[0] = m_Drive[0];
            pStr[1] = ':';
            pStr[2] = '\0';
        }

        // Copy the directory, if provided
        if (m_Dir[0] != '\0')
        {
            drgString::Concatenate(pStr, m_Dir);
            // Do we need to append a trailing directory separator?
            int iLen = (int)drgString::Length(pStr);

            if (pStr[iLen-1] != '\\' && pStr[iLen-1] != '/')
            {
                pStr[iLen+1] = '\0';
                pStr[iLen] = '/'; // If so, append the slash
            }
        }

        // Copy the subdirectory, if provided
        if (m_SubDir[0] != '\0')
        {
            drgString::Concatenate(pStr, m_SubDir);
            // Do we need to append a trailing directory separator?
            int iLen = (int)drgString::Length(pStr);

            if (pStr[iLen-1] != '\\' && pStr[iLen-1] != '/')
            {
                pStr[iLen+1] = '\0';
                pStr[iLen] = '/'; // If so, append the slash
            }
        }

		// If they want the filename
		if (bIncludeFile)
		{
			// Copy the filename, if provided
			if (m_Fname[0] != '\0')
				drgString::Concatenate(pStr, m_Fname);

			// Copy the extension, if provided
			if (m_Ext[0] != '\0')
			{   // Do we need to prepend a period?
				if (m_Ext[0] != '.')
					drgString::Concatenate(pStr, ".");
				drgString::Concatenate(pStr, m_Ext);
			}
		}
    }
}

void drgFilename::SetCaseFilename( bool upper )
{
	SetCase(m_Fname,upper);
}

void drgFilename::SetCaseExt( bool upper )
{
	SetCase(m_Ext,upper);
}

void drgFilename::SetCaseDrive( bool upper )
{
	SetCase(m_Drive,upper);
}

void drgFilename::SetCaseDir( bool upper )
{
	SetCase(m_Dir,upper);
}

void drgFilename::SetCasePlatformSubDir( bool upper )
{
	SetCase(m_SubDir,upper);
}

void drgFilename::SetCase( char* str, bool upper )
{
	if(upper)
		drgString::ToUpper(str);
	else
		drgString::ToLower(str);
}
