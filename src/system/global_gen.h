#ifndef __DRG_GLOBAL_GEN_H__
#define __DRG_GLOBAL_GEN_H__

#include <stdio.h>
#include <ctype.h>
#include <stdarg.h>


#define DRGTEXT char
#define DRGT(x) __T(x)

#define DRG_CALLBACK		__stdcall
#define DRG_PTR_ADD_PAD

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


#define ConvertUI64(x)	(*x)
#define ConvertI64(x)	(*x)


#endif // __DRG_GLOBAL_WIN_H__
