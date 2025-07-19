#ifndef __DRG_STRING_H__
#define __DRG_STRING_H__

#include "core/memmgr.h"

//!Class that provides operations for strings
class drgString
{
public:
    drgString();
	~drgString();

	static int   AtoI( const char* string );
	static float AtoF( const char* string );
	static char* ItoA( int val, char* string, int base );
	static char* FtoA(float val, char* string);

	static int CharToLower( int ch );
	static int CharToUpper( int ch );
	static char* ToLower(char* ch);
	static char* ToUpper(char* ch);

	static char* AllocCopy(char* src);
	static char* AllocCopy(char* src, int size);
	static void  ReplaceOrAllocCopy(char** dst, char* src);
	static void  ReplaceOrAllocCopy(char** dst, char* src, int size);

	static char* Copy( char* string, const char* istring );
	static char* CopySafe( char* string, const char* istring );
	static char* Copy( char* string, const char* istring, unsigned int size );
	static char* CopySafe( char* string, const char* istring, unsigned int size );
	static char* Concatenate( char* string, const char* istring );
	static int Compare( const char* string, const char* stringc );
	static int Compare( const char* string, const char* stringc, unsigned int numch );
	static int CompareNoCase( const char* string, const char* stringc );
	static int CompareNoCase( const char* string, const char* stringc, unsigned int numch );
	static const char *FindFirst( const char* string, int ch );
	static const char *FindLast( const char* string, int ch );
	static unsigned int Length( const char* string ); 
	static int PrintFormated( char *string, const char* fmt, ... );
	static int ScanFormated(const char *string, const char* fmt, ...);
	static void FormatLineEnd(char *string, const char* istring);
};

//!Class that copies a string and then locates substrings within it
class drgStringParse
{
public:
	drgStringParse();
	drgStringParse(const char* string, int parsechar = 32);
	~drgStringParse();

	void Cleanup();

	void SetParseString(const char* string, int parsechar = 32);

	inline int GetNumSubstrings() { return m_iNumSubstrings;}
	char* GetSubstring(int index);

private:
	char*	m_String;
	char**	m_StringPtrs;
	int		m_iNumSubstrings;
	
};

//!Class that provides operations for strings
class drgStringUni
{
public:
    drgStringUni();
	~drgStringUni();

	static short* Copy( short* string, const short* istring );
	static short* CopySafe( short* string, const short* istring );
	static short* Copy( short* string, const short* istring, unsigned int size );
	static short* Concatenate( short* string, const short* istring );
	static int Compare( const short* string, const short* stringc );
	static int Compare( const short* string, const short* stringc, unsigned int numch );
	static const short *FindFirst( const short* string, int ch );
	static const short *FindLast( const short* string, int ch );
	static unsigned int Length( const short* string ); 
	static short* InsertString( short* string, const short* istring1, const short* istring2 );
	static short* ItoU( int num, short* string );
};



#endif // __DRG_STRING_H__

