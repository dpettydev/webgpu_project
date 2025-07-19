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

inline long long ConvertI64(long long* ptr)
{
	long long ptrval = 0;
	unsigned char* cptr = (unsigned char*)ptr;
	ptrval |= (((long long)cptr[7])<<56);
	ptrval |= (((long long)cptr[6])<<48);
	ptrval |= (((long long)cptr[5])<<40);
	ptrval |= (((long long)cptr[4])<<32);
	ptrval |= (((long long)cptr[3])<<24);
	ptrval |= (((long long)cptr[2])<<16);
	ptrval |= (((long long)cptr[1])<<8);
	ptrval |= (((long long)cptr[0])<<0);
	return ptrval;
}

inline unsigned long long ConvertUI64(unsigned long long* ptr)
{
	unsigned long long ptrval = 0;
	unsigned char* cptr = (unsigned char*)ptr;
	ptrval |= (((unsigned long long)cptr[7])<<56);
	ptrval |= (((unsigned long long)cptr[6])<<48);
	ptrval |= (((unsigned long long)cptr[5])<<40);
	ptrval |= (((unsigned long long)cptr[4])<<32);
	ptrval |= (((unsigned long long)cptr[3])<<24);
	ptrval |= (((unsigned long long)cptr[2])<<16);
	ptrval |= (((unsigned long long)cptr[1])<<8);
	ptrval |= (((unsigned long long)cptr[0])<<0);
	return ptrval;
}


#endif // __DRG_GLOBAL_WIN_H__
