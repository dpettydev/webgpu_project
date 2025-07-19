#ifndef __DRG_MEMMGR_H__
#define __DRG_MEMMGR_H__

#include "core/memblocks.h"

#define SAFE_DELETE( x )			if(x){ delete x; x = NULL; }
#define SAFE_DELETE_ARRAY( x )		if( x ) { delete[] x; x = NULL; }
#define SAFE_FREE( x )				if( x ) { drgMemFree( x ); x = NULL; }

#define SAFE_FREE_ARRAY_OF_ALLOCS( x, count )	if( x ){ for( int i = 0; i < count; ++i ) { SAFE_FREE( x[i] ); } SAFE_FREE( x ); }
#define SAFE_FREE_ARRAY_OF_NEWS( x, count )	if( x ){ for( int i = 0; i < count; ++i ) { SAFE_DELETE( x[i] ); } SAFE_FREE( x ); }

class drgMemory
{
public:
	drgMemory();
	~drgMemory();

	static void InitMainSpace(){};
	static void DestroyMainSpace(){};
	static inline bool IsInitialized() {return true;}
	static inline void* MemSet( void* mem, int val, unsigned int size );
	static inline void* MemCopy( void* mem, void* val, unsigned int size );
	static inline void* MemMove( void* mem, void* val, unsigned int size );
	static inline int MemCompare( const void* mem, const void* val, unsigned int size );
};



#define drgAllocA(mem,file,line) alloca(mem)

#include <string.h>
inline void* drgMemory::MemSet( void* mem, int val, unsigned int size ) {return memset(mem, val, size);};
inline void* drgMemory::MemCopy( void* mem, void* val, unsigned int size ) {return memcpy(mem, val, size);};
inline void* drgMemory::MemMove( void* mem, void* val, unsigned int size ) {return memmove(mem, val, size);};
inline int drgMemory::MemCompare( const void* mem, const void* val, unsigned int size ) {return memcmp(mem, val, size);};


#endif // __DRG_MEMMGR_H__

