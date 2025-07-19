#ifndef __DRG_GLOBAL_WIN_H__
#define __DRG_GLOBAL_WIN_H__

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>


#define CALLBACK    __stdcall
#define WINAPI      __stdcall
#define WINAPIV     __cdecl
#define APIENTRY    WINAPI
#define APIPRIVATE  __stdcall
#define PASCAL      __stdcall
#ifndef WINGDIAPI
	#define WINGDIAPI
#endif


typedef char				drgInt8;
typedef short				drgInt16;
typedef int					drgInt32;
typedef long long			drgInt64;

typedef unsigned char		drgUInt8;
typedef unsigned short		drgUInt16;
typedef unsigned int		drgUInt32;
typedef unsigned long long	drgUInt64;


#ifdef USE_64_BIT
typedef unsigned long long	drgIntPt;  // use for pointer calcs
#else
typedef unsigned int		drgIntPt;  // use for pointer calcs
#endif

#ifdef INT64_FMT
	#undef INT64_FMT
#endif
#ifdef UINT64_FMT
	#undef UINT64_FMT
#endif
#define INT64_FMT  "I64d"
#define UINT64_FMT  "I64u"

#define ConvertUI64(x)	(*x)
#define ConvertI64(x)	(*x)

#endif // __DRG_GLOBAL_WIN_H__
