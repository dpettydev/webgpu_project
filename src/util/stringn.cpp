
#include "util/stringn.h"
#include "system/global.h"
#include <wchar.h>

#pragma warning(disable:4996) // Disable warning for deprecation

//!Default constrcutor
drgString::drgString()
{
}

//!Destruction
drgString::~drgString()
{
}

//!Function that converts a string to an integer.
/*!
	\param string string to convert
	\returns the int value
*/
int drgString::AtoI( const char* string )
{
	if(!string)
		return 0;
	int limit = DRG_INT_MAX / 10;
	int state = 0;
	bool sign = true;
	int value = 0;
	int ch;

	while (state != 2)
	{
		ch = (unsigned char)*string++;

		switch (state)
		{
		case 0: // skip white space
			if (isspace(ch))
			{
				;
			}

			else if (ch == '-')
			{
				state = 1; // get digits
				sign = false; // negative
			}
			else if (ch == '+')
			{
				state = 1; // get digits
			}
			else if (isdigit(ch))
			{
				state = 1; // get digits
				value = ch - '0';
			}

			else
			{
				state = 2; // finished;
			}
			break;

		case 1: // get_digits
			if (isdigit(ch))
			{
				ch -= '0';
				if ((value < limit) ||
					((DRG_INT_MAX - value * 10) >= ch))
				{
					value *= 10;
					value += ch;
				}
				else
				{
					value = DRG_INT_MAX;
					state = 2; // finished
				}
			}
			else
			{
				state = 2; // finished
			}
			break;

		case 2: // finished
		default:
			break;

		}     // end of switch (state)
	}     // end of while (state != FINISHED)

	// now see if there was a negative sign
	if (sign == false)
	{
		value *= -1;
	}

	return value;
}

//!Function that converts a string to a float.
/*!
	\param string string to convert
	\returns the float value
*/
float drgString::AtoF( const char* string )
{
	if(!string)
		return 0.0f;
	float retval=0.0f;
	sscanf(string, "%f", &retval);
	return retval;
}

//!Function that converts a integer to a string.
char* drgString::ItoA( int val, char* string, int base )
	{
	if(base == 16)
		sprintf( string, "%x", val);
	else if(base == 8)
		sprintf( string, "%o", val);
	else
		sprintf( string, "%i", val );
		return string;
	}
	
//!Function that converts a float to a string.
char* drgString::FtoA( float val, char* string )
	{
	sprintf( string, "%g", val );
	return string;
}


//!Function that copies the contents one string to another.
/*
	\param *string destination string
	\param *istring source string
	\returns char* containing the destination string
*/
char* drgString::Copy( char* string, const char* istring )
{
	return strcpy( string, istring );
}

//!Function that copies the contents one string to another.
/*
	\param *string destination string
	\param *istring source string
	\returns char* containing the destination string
*/
char* drgString::CopySafe( char* string, const char* istring )
{
	if(!string)
		return NULL;
	if(!istring)
		return strcpy( string, "" );
	return strcpy( string, istring );
}

//!Function that copies the first n characters of one string to another.
/*
	\param string destination string
	\param istring source string
	\param size number of characters to copy
	\returns char* containing the destination string
*/
char* drgString::Copy( char* string, const char* istring, unsigned int size )
{
	return strncpy( string, istring, size );
}

char* drgString::CopySafe( char* string, const char* istring, unsigned int size )
{
	if(!string)
		return NULL;
	if(!istring)
		return strcpy( string, "" );

	char *ret=strncpy( string, istring, size );
	return ret;
}

//!Concatenates the contents of one string to the end of another string
/*!
	The contents of the source string will be concatenated to the end of the destination string
	and all will be stored in the destination string.
	\param *string destination string
	\param *istring source string
	\returns char* containing the destination string
*/
char* drgString::Concatenate( char* string, const char* istring )
{
	return strcat( string, istring );
}

//!Function that compares the contents of two strings
/*!
	\param *string first string to compare
	\param *stringc second string to compare
	\returns <0 if string<stringc
	\returns 0 if string=stringc
	\returns >0 if string>stringc
*/
int drgString::Compare( const char* string, const char* stringc )
{
	return strcmp( string, stringc );
}

//!Function that compares the contents of two strings up to n number of characters
/*!
	\param *string first string to compare
	\param *stringc second string to compare
	\param numch number of characters to compare up to
	\returns <0 if string<stringc
	\returns 0 if string=stringc
	\returns >0 if string>stringc
*/
int drgString::Compare( const char* string, const char* stringc, unsigned int numch )
{
	return strncmp( string, stringc, numch );
}

//!Function that compares the contents of two strings ignoring case
/*!
	\param *string first string to compare
	\param *stringc second string to compare
	\returns <0 if string<stringc
	\returns 0 if string=stringc
	\returns >0 if string>stringc
*/
int drgString::CompareNoCase( const char* string, const char* stringc )
{
#if defined(_PS3) || defined(_LLVM)
	return strcasecmp( string, stringc );
#elif defined(_DS) || defined(_WINMO) || defined(_IPHONE) || defined(_MAC) || defined(_LINUX) || defined(_ANDROID)
	unsigned char c1,c2;
	do {
		c1 = *string++;
		c2 = *stringc++;
		c1 = (unsigned char) tolower( (unsigned char) c1);
		c2 = (unsigned char) tolower( (unsigned char) c2);
	}
	while((c1 == c2) && (c1 != '\0'));
	return (int) c1-c2;
#else	
	if( (string == NULL) != (stringc == NULL) )
		return false;
	return stricmp( string, stringc );
#endif
}

//!Function that compares the contents of two strings up to n number of characters ignoring case
/*!
	\param *string first string to compare
	\param *stringc second string to compare
	\param numch number of characters to compare up to
	\returns <0 if string<stringc
	\returns 0 if string=stringc
	\returns >0 if string>stringc
*/
int drgString::CompareNoCase( const char* string, const char* stringc, unsigned int numch )
{
#if defined(_PS3) || defined(_LLVM)
	return strncasecmp( string, stringc, numch );
#elif defined(_DS) || defined(_WINMO) || defined(_IPHONE) || defined(_MAC) || defined(_LINUX) || defined(_ANDROID)
	unsigned char c1,c2;
	unsigned int cnt=0;
	do {
	c1 = *string++;
	c2 = *stringc++;
	c1 = (unsigned char) tolower( (unsigned char) c1);
	c2 = (unsigned char) tolower( (unsigned char) c2);
	++cnt;
	}
	while((c1 == c2) && (c1 != '\0') && (cnt<numch));
	return (int) c1-c2;
#else
	if( (string == NULL) != (stringc == NULL) )
		return false;
	return strnicmp( string, stringc, numch );
#endif
}

//!Function that returns a pointer to the first occurrence of a character in a string
/*!
	\param *string string to search
	\param ch character to search for
	\returns a pointer to the first occurrence of the character in the string
*/
const char *drgString::FindFirst( const char* string, int ch )
{
	return strchr( string, ch );
}

//!Function that returns a pointer to the last occurrence of a character in a string
/*!
	\param *string string to search
	\param ch character to search for
	\returns a pointer to the last occurrence of the character in the string
*/
const char *drgString::FindLast( const char* string, int ch )
{
	return strrchr( string, ch );
}

//!Function that returns the length of the input string
/*!
	\param string input string
	\returns the length of the input
*/
unsigned int drgString::Length( const char* string )
{
	return (unsigned int)strlen( string );
}

//!Function that returns the lowercase version of the input character.
/*!
	\param ch
	\returns the lowercase version of ch, or ch if it is allready lowercase.
*/
int drgString::CharToLower( int ch )
{
	return tolower( ch );
}

//!Function that returns the uppercase version of the input character.
/*!
	\param ch
	\returns the uppercase version of ch, or ch if it is allready uppercase.
*/
int drgString::CharToUpper( int ch )
{
	return toupper( ch );
}

char* drgString::ToLower(char* string)
{
	int count = 0;
	for(; string[count]!='\0'; ++count )
		string[count]=drgString::CharToLower(string[count]);
	string[count]='\0';
	return string;
}

char* drgString::ToUpper(char* string)
{
	int count = 0;
	for(; string[count]!='\0'; ++count )
		string[count]=drgString::CharToUpper(string[count]);
	string[count]='\0';
	return string;
}

//!Function that creates a formatted string using a format string and a list of arguments
/*!
	\param *string output string to create
	\param *fmt format of the output string
	\param ... list of arguments
	\returns the number of characters in the output string
*/
int drgString::PrintFormated(char *string, const char* fmt, ...)
{
	if (!string)
		return 0;
	va_list argp;
	va_start(argp, fmt);
	int ret = vsprintf(string, fmt, argp);
	va_end(argp);
	return ret;
}

int drgString::ScanFormated(const char *string, const char* fmt, ...)
{
	if (!string)
		return 0;
	va_list argp;
	va_start(argp, fmt);
	int ret = vsscanf(string, fmt, argp);
	va_end(argp);
	return ret;
}

void drgString::FormatLineEnd(char *string, const char* istring)
{
	if(!istring)
		return;
	if(!string)
		return;
	unsigned int count1=0;
	unsigned int count2=0;
	unsigned int length=Length(istring);
	char curchar;
	for(; count1<length; ++count1)
	{
		if(istring[count1]=='\r' || istring[count1]=='\n')
		{
			curchar=istring[count1];
			while((curchar!=istring[count1+1]) && (istring[count1+1]=='\r' || istring[count1+1]=='\n'))
				++count1;
			string[count2++]='\n';
		}
		else
		{
			string[count2++]=istring[count1];
		}
	}
	string[count2]='\0';
}


//!Default Constructor for drgStringParse
/*!
	Initialized variables, must use SetParseString if using default constructor
*/
drgStringParse::drgStringParse()
{
	m_String = NULL;		// set these to NULL right away in case we destruct
	m_StringPtrs = NULL;
	m_iNumSubstrings = 0;
}

//!Constructor for drgStringParse
/*!
	\param *string string to copy and reference
	\param parsechar character to parse against (default = SPACE (32))
*/
drgStringParse::drgStringParse(const char* string, int parsechar)
{
	m_String = NULL;		// set these to NULL right away in case we destruct
	m_StringPtrs = NULL;
	m_iNumSubstrings = 0;

	// Speed things up a bit by doing this only once.
	int iStrLen = drgString::Length(string);

	assert( string);		// the input string doesn't exist
	assert( iStrLen > 0);	// the input string is empty
	assert( parsechar);		// the parsechar is NULL

	// Allocate memory to copy the string internally. We do this because we're
	// going to create pointers into that string and we need to change it to
	// create mini-strings that are null-terminated.
	m_String = (char*)drgMemAlloc(iStrLen + 1);
	drgMemory::MemCopy((void*)m_String, (void*)string, iStrLen);
	m_String[iStrLen] = 0;

	// First let's find out how many substrings we have
	char* ptr = m_String;
	if( *ptr != (char) parsechar)	// The string starts off with a valid ministring
		m_iNumSubstrings++;
	while(*ptr)					// we're counting on a null terminated string
	{
		if( ((*ptr) == (char)parsechar) &&							// The character we're on in one of our parsechars
			((drgUInt64)(ptr+1) < ((drgUInt64)m_String+iStrLen)) &&	// We're not at the end of the string
			(*(ptr+1) != (char)parsechar) )							// The next character is not one of our parsechars
				m_iNumSubstrings++;
		ptr++;
	}
	if( !m_iNumSubstrings) return;									// There are no substrings to parse

	m_StringPtrs = (char**)drgMemAlloc(sizeof(char*) * m_iNumSubstrings);	// get our buffer for pointers set up

	// Now let's parse our substrings
	m_iNumSubstrings = 0;
	ptr = m_String;
	if( *ptr != (char) parsechar)	// The string starts off with a valid ministring
	{
		m_StringPtrs[m_iNumSubstrings] = ptr;
		m_iNumSubstrings++;
	}
	while(*ptr)					// we're counting on a null terminated string
	{
		if( (*ptr) == (char)parsechar)		// The character we're on in one of our parsechars
		{
			if( ((drgUInt64)(ptr+1) < ((drgUInt64)m_String+iStrLen)) &&	// We're not at the end of the string
				(*(ptr+1) != (char)parsechar))							// The next character is not one of our parsechars
			{
				m_StringPtrs[m_iNumSubstrings] = ptr+1;
				m_iNumSubstrings++;
			}
			*ptr = 0;					// NULL out any parsechars so our mini-strings are all terminated
		}
		ptr++;
	}
}

//!Destructor for drgStringParse class
drgStringParse::~drgStringParse()
{
	Cleanup();
}

void drgStringParse::Cleanup()
{
	if( m_String)  // we must have allocated some memory
	{
		drgMemFree(m_String);  // clear it
		m_String = NULL;
	}

	if( m_StringPtrs) // we allocated some char ptrs
	{
		drgMemFree(m_StringPtrs);  // clear it
		m_StringPtrs = NULL;
	}
	m_iNumSubstrings = 0;
}


//!Allows class to be reused on another string
/*!
	\param *string string to copy and reference
	\param parsechar character to parse against (default = SPACE (32))
*/
void drgStringParse::SetParseString(const char* string, int parsechar)
{
	Cleanup();

	// Speed things up a bit by doing this only once.
	int iStrLen = drgString::Length(string);

	assert( string);		// the input string doesn't exist
	assert( iStrLen > 0);	// the input string is empty
	assert( parsechar);		// the parsechar is NULL

	// Allocate memory to copy the string internally. We do this because we're
	// going to create pointers into that string and we need to change it to
	// create mini-strings that are null-terminated.
	m_String = (char*)drgMemAlloc(iStrLen + 1);
	drgMemory::MemCopy((void*)m_String, (void*)string, iStrLen);
	m_String[iStrLen] = 0;

	// First let's find out how many substrings we have
	char* ptr = m_String;
	if( *ptr != (char) parsechar)	// The string starts off with a valid ministring
		m_iNumSubstrings++;
	while(*ptr)					// we're counting on a null terminated string
	{
		if( ((*ptr) == (char)parsechar) &&							// The character we're on in one of our parsechars
			((drgUInt64)(ptr+1) < ((drgUInt64)m_String+iStrLen)) &&	// We're not at the end of the string
			(*(ptr+1) != (char)parsechar) )							// The next character is not one of our parsechars
				m_iNumSubstrings++;
		ptr++;
	}
	if( !m_iNumSubstrings) return;									// There are no substrings to parse

	m_StringPtrs = (char**)drgMemAlloc(sizeof(char*) * m_iNumSubstrings);	// get our buffer for pointers set up

	// Now let's parse our substrings
	m_iNumSubstrings = 0;
	ptr = m_String;
	if( *ptr != (char) parsechar)	// The string starts off with a valid ministring
	{
		m_StringPtrs[m_iNumSubstrings] = ptr;
		m_iNumSubstrings++;
	}
	while(*ptr)					// we're counting on a null terminated string
	{
		if( (*ptr) == (char)parsechar)		// The character we're on in one of our parsechars
		{
			if( ((drgUInt64)(ptr+1) < ((drgUInt64)m_String+iStrLen)) &&	// We're not at the end of the string
				(*(ptr+1) != (char)parsechar))							// The next character is not one of our parsechars
			{
				m_StringPtrs[m_iNumSubstrings] = ptr+1;
				m_iNumSubstrings++;
			}
			*ptr = 0;					// NULL out any parsechars so our mini-strings are all terminated
		}
		ptr++;
	}
}

//!Function for getting a substring of a parsed string
/*!
	\param index index of the substring to return
	\returns a pointer to the substring
*/
char* drgStringParse::GetSubstring(int index)
{
	if(index < m_iNumSubstrings)		// Is this index in the valid range?
		return m_StringPtrs[index];		// return it

	return NULL;						// otherwise return NULL
}


// Unicode strings
//!Default constrcutor
drgStringUni::drgStringUni()
{
}

//!Destruction
drgStringUni::~drgStringUni()
{
}

//!Function that copies the contents one string to another.
/*
	\param *string destination string
	\param *istring source string
	\returns short* containing the destination string
*/
short* drgStringUni::Copy( short* string, const short* istring )
{
	int i;
	for (i=0; istring[i]!=0; ++i)
	{
		string[i] = istring[i];
	}
	string[i]=0;

	return (string);
}

//!Function that copies the contents one string to another.
/*
	\param *string destination string
	\param *istring source string
	\returns short* containing the destination string
*/
short* drgStringUni::CopySafe( short* string, const short* istring )
{
	if(!string)
		return NULL;

	if(!istring)
	{
		string[0] = 0;
		return string;
	}

	Copy(string,istring);

	return string;
}

//!Function that copies the first n characters of one string to another.
/*
	\param string destination string
	\param istring source string
	\param size number of characters to copy
	\returns char* containing the destination string
*/
short* drgStringUni::Copy( short* string, const short* istring, unsigned int size )
{
	unsigned int i;
	for (i=0; istring[i]!=0 && i<size-1; ++i)
	{
		string[i] = istring[i];
	}
	string[i]=0;

	return (string);
}

//!Concatenates the contents of one string to the end of another string
/*!
	The contents of the source string will be concatenated to the end of the destination string
	and all will be stored in the destination string.
	\param *string destination string
	\param *istring source string
	\returns short* containing the destination string
*/
short* drgStringUni::Concatenate( short* string, const short* istring )
{
	// this doesn't work on psp
	//return (short*)wcscat( (wchar_t*)string, (wchar_t*)istring );

	// do it manually
	int firstEndPos = -1;
	for (int i=0;;++i)
	{
		if (string[i]==0)
		{
			firstEndPos = i;
			break;
		}
	}
	for (int i=firstEndPos,j=0;;++i,++j)
	{
		string[i] = istring[j];
		if (istring[j]==0)
		{
			string[i+1] = 0;
			break;
		}
	}

	return (string);
}

//!Function that compares the contents of two strings
/*!
	\param *string first string to compare
	\param *stringc second string to compare
	\returns <0 if string<stringc
	\returns 0 if string=stringc
	\returns >0 if string>stringc
*/
int drgStringUni::Compare( const short* string, const short* stringc )
{
#if (defined(_ANDROID) || defined(_IPHONE))
	unsigned short c1,c2;
	do {
	c1 = *string++;
	c2 = *stringc++;
	}
	while((c1 == c2) && (c1 != 0));
	return (int) c1-c2;
#else
	return wcscmp( (wchar_t*)string, (wchar_t*)stringc );
#endif
}

//!Function that compares the contents of two strings up to n number of shortacters
/*!
	\param *string first string to compare
	\param *stringc second string to compare
	\param numch number of characters to compare up to
	\returns <0 if string<stringc
	\returns 0 if string=stringc
	\returns >0 if string>stringc
*/
int drgStringUni::Compare( const short* string, const short* stringc, unsigned int numch )
{
#if (defined(_ANDROID) || defined(_IPHONE))
	unsigned int count = 0;
	unsigned short c1,c2;
	do {
	c1 = *string++;
	c2 = *stringc++;
	count++;
	}
	while((c1 == c2) && (c1 != 0) && count < numch);
	return (int) c1-c2;
#else
	return wcsncmp( (wchar_t*)string, (wchar_t*)stringc, numch );
#endif
}

//!Function that returns a pointer to the first occurrence of a character in a string
/*!
	\param *string string to search
	\param ch character to search for
	\returns a pointer to the first occurrence of the character in the string
*/
const short *drgStringUni::FindFirst( const short* string, int ch )
{
#if (defined(_ANDROID) || defined(_IPHONE))
	drgPrintError( "Wide char functions are not implemented in Android, you'll have to write this one yourself" );
#endif
	return (short*)wcschr( (wchar_t*)string, ch );
}

//!Function that returns a pointer to the last occurrence of a character in a string
/*!
	\param *string string to search
	\param ch character to search for
	\returns a pointer to the last occurrence of the character in the string
*/
const short *drgStringUni::FindLast( const short* string, int ch )
{
#if (defined(_ANDROID) || defined(_IPHONE))
	drgPrintError( "Wide char functions are not implemented in Android, you'll have to write this one yourself" );
#endif
	return (short*)wcsrchr( (wchar_t*)string, ch );
}

//!Function that returns the length of the input string
/*!
	\param string input string
	\returns the length of the input
*/
unsigned int drgStringUni::Length( const short* string )
{
	unsigned int numChars = 0;
	while( *(string++) )
	{
		numChars++;
	}

	return numChars;
}

//!Function that returns the string with a string inserted into the string
short* drgStringUni::InsertString( short* string, const short* istring1, const short* istring2 )
{
	int strpos1=0;

	for(;;)
	{
		string[strpos1]=istring1[strpos1];
		if(istring1[strpos1]=='\0')
			return string;
		if (istring1[strpos1]=='%' && istring1[strpos1+1]=='s')
			break;
		++strpos1;
	}

	Copy(&(string[strpos1]), istring2);
	Concatenate(string, &(istring1[strpos1+2]));
	return string;
}

//!Function that converts a integer to an unicode string
/*!
	\param num the number to convert
	\param string the converted string
*/
short* drgStringUni::ItoU( int num, short* string )
{
	char cNum[11];
	drgString::PrintFormated(cNum, "%i", num);

	unsigned int i;
	for(i = 0; i < drgString::Length(cNum); ++i)
		string[i] = (short)cNum[i];
	string[i] = 0;
	string[i+1] = 0;

	return string;
}
