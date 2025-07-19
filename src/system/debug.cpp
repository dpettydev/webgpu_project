
#include "system/extern_inc.h"
#include "system/debug.h"
#include "system/global.h"

#include <stdio.h>
#include <stdarg.h>

static DRG_DEBUG_FAIL_TYPE g_FailType = DRG_DEBUG_FAIL_NONE;

static drgMutex m_PrintMutex;

extern int printf(const char* fmt, ...);
extern "C" void drgLibPrint( const char* buff );

extern void set_debug_traps();
extern void breakpoint();

void drgDebug::Init()
{
	//set_debug_traps();
	//breakpoint();
}

#if !defined(RETAIL_BUILD)

#if defined(_WINDOWS)
bool g_WinConsole = false;
#endif


void drgDebug::Notify(const char *fmt, ... )
{
	char buff[1024];
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buff, fmt, argp);
	va_end(argp);

#if defined(_WINDOWS)
	MessageBoxA(NULL, buff, "Notify!", MB_OK);
#elif defined(_ANDROID)
	__android_log_write(ANDROID_LOG_INFO, "ACE", buff);
#elif defined(_IPHONE)
    printf("ACE - NOTIFY: %s", buff);
#else
	printf("Notify: %s", buff);
#endif
}

void drgDebug::Error(const char *fmt, ... )
{
	char buff[4096+1024];
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buff, fmt, argp);
	va_end(argp);

#if defined(_WINDOWS)
    printf("Error: %s", buff);
	MessageBoxA(NULL, buff, "Error!!", MB_OK);
#elif defined(_ANDROID)
	__android_log_write(ANDROID_LOG_INFO, "ACE", buff);
#elif defined(_IPHONE)
    printf("Error: %s", buff);
#else
	printf("Error: %s", buff);
#endif
	
	RETAIL_IN(exit(0));
	*(int *)0=0;
}

void drgDebug::Print(const char *buff, int number)
{
	m_PrintMutex.Lock();
	int size=0;
#if defined(_WINDOWS)
	if(g_WinConsole)
	{
		static HANDLE cnslout = 0;
		DWORD count ;
		if( cnslout == 0)
			cnslout = GetStdHandle( STD_OUTPUT_HANDLE );
		WriteConsole(cnslout, buff, (DWORD)strlen(buff), &count, NULL );
	}
	else
	{
		OutputDebugStringA(buff);
	}
	printf(buff);

#elif defined(EMSCRIPTEN)
	printf(buff);
	drgLibPrint(buff);
#elif defined(_ANDROID)
	__android_log_write(ANDROID_LOG_INFO, "ACE", buff);
#elif defined(_IPHONE)
    printf("ACE - %s", buff);
#else
	printf(buff);
#endif
	m_PrintMutex.Unlock();
}

void drgPrintError( const char *fmt, ... )
{
    char	buff[2048];
	char*	buffptr;

	sprintf(buff, "-- ERROR -- ");	// Add something to the beginning so we can pick these messages out
	buffptr = &buff[12];				// to make room for the ERROR addition above

	va_list argp;
	va_start(argp, fmt);
	vsprintf(buffptr, fmt, argp);
	va_end(argp);

	drgDebug::Print(buff, 2);
}

void drgPrintWarning( const char *fmt, ... )
{
    char	buff[2048];
	char*	buffptr;

	sprintf(buff, "-- WARNING -- ");	// Add something to the beginning so we can pick these messages out
	buffptr = &buff[14];				// to make room for the ERROR addition above

	va_list argp;
	va_start(argp, fmt);
	vsprintf(buffptr, fmt, argp);
	va_end(argp);

	drgDebug::Print(buff, 1);
}

extern "C" void drgPrintOut( const char *fmt, ... )
{
	char buff[2048];
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buff, fmt, argp);
	va_end(argp);

	drgDebug::Print(buff);
}

extern "C" void drgPrintF(int num, const char *fmt, ... )
{
	char buff[2048];
	va_list argp;
	va_start(argp, fmt);
	vsprintf(buff, fmt, argp);
	va_end(argp);

	drgDebug::Print(buff, num);
}

void _assert_drg(const char *file, unsigned int line, const char *assertion)
{
	drgDebug::Error("Assertion \n\"%s\" failed in %s:%d\n",assertion,file,line);
}

#endif


DRG_DEBUG_FAIL_TYPE drgDebugGetFailType()
{
	return g_FailType;
}

void drgDebugReSetFailType()
{
	g_FailType = DRG_DEBUG_FAIL_NONE;
}

void drgDebugSetFailType( DRG_DEBUG_FAIL_TYPE type )
{
	g_FailType = type;
}
