#ifndef __DRG_MEMORY_BLOCK_H__
#define __DRG_MEMORY_BLOCK_H__

#if defined(_IPHONE)
    #define USE_MEM_MANAGER 	0
#else
    #define USE_MEM_MANAGER 	0
#endif

#define _NEW_
#if defined(__GNUC__)
#define MEM_OVERLOAD_NEW		0
#else
#define MEM_OVERLOAD_NEW		0
#endif
#define MEM_USE_SMALL_BLOCKS	0

#if USE_MEM_MANAGER
#define drgMemAllocZero(size) g_MainBlocks.ReserveZero(size, 32 MEM_INFO_AUTO)
#define drgMemAlloc(size) g_MainBlocks.Reserve(size, 32 MEM_INFO_AUTO)
#define drgMemAlign(align,size) g_MainBlocks.Reserve(size, align MEM_INFO_AUTO)
#define drgMemCAlloc(elements,size) g_MainBlocks.ReserveZero(size*elements, 32 MEM_INFO_AUTO)
#define drgMemReAlloc(mem,size) g_MainBlocks.ReReserve(mem, size, 32 MEM_INFO_AUTO)
#define drgMemFree(mem) g_MainBlocks.Release(mem)

#define drgMemAllocDynamic(size) g_MainBlocks.Reserve(size, 32 MEM_INFO_AUTO)
#define drgMemAlignDynamic(align,size) g_MainBlocks.Reserve(size, align MEM_INFO_AUTO)
#define drgMemCAllocDynamic(elements,size) g_MainBlocks.ReserveZero(size*elements, 32 MEM_INFO_AUTO)
#define drgMemReAllocDynamic(mem,size) g_MainBlocks.ReReserve(mem, size, 32 MEM_INFO_AUTO)
#define drgMemFreeDynamic(mem) g_MainBlocks.Release(mem)
#else
extern void* malloc_zero(unsigned int size);
#define drgMemAllocZero(size) malloc_zero(size)
#define drgMemAlloc(size) malloc(size)
#define drgMemAlign(align,size) malloc(size)
#define drgMemCAlloc(elements,size) malloc_zero(size*elements)
#define drgMemReAlloc(mem,size) realloc(mem, size)
#define drgMemFree(mem) free(mem)

#define drgMemAllocDynamic(size) malloc(size)
#define drgMemAlignDynamic(align,size) malloc(size)
#define drgMemCAllocDynamic(elements,size) malloc_zero(size*elements
#define drgMemReAllocDynamic(mem,size) realloc(mem, size)
#define drgMemFreeDynamic(mem) free(mem)
#endif

#define _NEW_

#if defined(RETAIL_BUILD)
#define MEM_ENABLE_CHECKS	0
#define MEM_ENABLE_DEBUG	0
#elif defined(_WII)
#define MEM_ENABLE_CHECKS	0
#define MEM_ENABLE_DEBUG	0
#elif defined(_3DS)
#define MEM_ENABLE_CHECKS	0
#define MEM_ENABLE_DEBUG	0
#elif defined(_IPHONE)
#define MEM_ENABLE_CHECKS	0
#define MEM_ENABLE_DEBUG	0
#else
#define MEM_ENABLE_CHECKS	0
#define MEM_ENABLE_DEBUG	0
#endif

#include <stdlib.h>
#include "system/debug.h"
#include "system/mutex.h"


#if defined(_WII)
	typedef unsigned long		drgIntSize;		// use for new overloads
	typedef unsigned int		drgIntAddr;		// use for address overloads
	#define CHUNK_PAD
	#define NEW_POST
	#define DEL_POST
#elif defined(_LINUX)
	#ifdef USE_64_BIT
		typedef long unsigned int	drgIntSize;		// use for new overloads
		typedef unsigned long long	drgIntAddr;		// use for address overloads
		#define CHUNK_PAD			char padval[8]
		#define NEW_POST
		#define DEL_POST
	#else
		typedef unsigned int		drgIntSize;		// use for new overloads
		typedef unsigned int		drgIntAddr;		// use for address overloads
		#define CHUNK_PAD
		#define NEW_POST
		#define DEL_POST
	#endif
#elif defined(_PS2)
	typedef unsigned int		drgIntSize;		// use for new overloads
	typedef unsigned int		drgIntAddr;		// use for address overloads
	#define CHUNK_PAD
	#define NEW_POST
	#define DEL_POST
#elif defined(_3DS)
	typedef unsigned int		drgIntSize;		// use for new overloads
	typedef unsigned int		drgIntAddr;		// use for address overloads
	#define CHUNK_PAD
	#define NEW_POST throw(std::bad_alloc)
	#define DEL_POST throw()
#elif defined(_IPHONE)
    typedef size_t				drgIntSize;		// use for new overloads
    typedef unsigned long long	drgIntAddr;		// use for address overloads
    #define CHUNK_PAD			char padval[8]
    #define NEW_POST
    #define DEL_POST
#else
	#ifdef USE_64_BIT
		typedef size_t				drgIntSize;		// use for new overloads
		typedef unsigned long long	drgIntAddr;		// use for address overloads
		#define CHUNK_PAD			char padval[8]
		#define NEW_POST
		#define DEL_POST
	#else
		typedef unsigned int		drgIntSize;		// use for new overloads
		typedef unsigned int		drgIntAddr;		// use for address overloads
		#define CHUNK_PAD
		#define NEW_POST
		#define DEL_POST
	#endif
#endif

#if MEM_OVERLOAD_NEW
#ifdef	new
#undef	new
#endif
#ifdef	delete
#undef	delete
#endif
#endif

#if USE_MEM_MANAGER
#ifdef	malloc
#undef	malloc
#endif

#ifdef	calloc
#undef	calloc
#endif

#ifdef	realloc
#undef	realloc
#endif

#ifdef	free
#undef	free
#endif

#ifdef  memalign
#undef  memalign
#endif
#endif

#if MEM_ENABLE_DEBUG
	#define MEM_INFO_FLAG_NEW			(1<<0)
	#define MEM_INFO_FLAG_MALLOC		(1<<1)
	#define MEM_INFO_FLAG_CHECKED		(1<<2)
	#define MEM_INFO_FLAG_PRINTED		(1<<3)
	#define MEM_INFO_FLAG_BLOCKS		(1<<4)
	#define MEM_INFO_FLAG_GAPS			(1<<5)
	#define MEM_INFO_FLAG_CHANGES		(1<<6)
	#define MEM_INFO_FLAG_CHECK_ONLY	(1<<7)
	#define MEM_INFO_FLAG_FREE			(1<<8)
	#define MEM_INFO_FLAG_JS_LEAKS		(1<<9)
	#define MEM_INFO_FLAG_USER_8		(1<<23)
	#define MEM_INFO_FLAG_USER_7		(1<<24)
	#define MEM_INFO_FLAG_USER_6		(1<<25)
	#define MEM_INFO_FLAG_USER_5		(1<<26)
	#define MEM_INFO_FLAG_USER_4		(1<<27)
	#define MEM_INFO_FLAG_USER_3		(1<<28)
	#define MEM_INFO_FLAG_USER_2		(1<<29)
	#define MEM_INFO_FLAG_USER_1		(1<<30)
	#define MEM_INFO_FLAG_USER_MASK		(0xFF000000)

	#define MEM_INFO_AUTO , __FILE__, __LINE__
	#define MEM_INFO_DECL , const char* desc, unsigned int linenum
	#define MEM_INFO_PASS , desc, linenum
	#define MEM_INFO_PASS_NEW , drgMemBlock::m_CurDescription, drgMemBlock::m_CurLineNumber
#if defined (_WINDOWS)
	#define MEM_INFO_LENGTH (88)
#else
	#define MEM_INFO_LENGTH (16)
#endif
	#define MEM_INFO_NEW_NUM (999999)
	#define MEM_INFO_SET_NEW drgMemBlock::SetOwner( __FILE__, (__LINE__)+1 )
#else
	#define MEM_INFO_AUTO
	#define MEM_INFO_DECL
	#define MEM_INFO_PASS
	#define MEM_INFO_PASS_NEW
	#define MEM_INFO_SET_NEW
#endif

#if MEM_USE_SMALL_BLOCKS
#define BLOCK_SMALL_SIZE	32
typedef struct
{
	unsigned char data[BLOCK_SMALL_SIZE-1];
	unsigned char size;
} SMALL_BLOCK;
#endif

enum DRG_MEM_BLOCK_TYPE
{
	DRG_MEM_MAIN = 0,
	DRG_MEM_FAST,
	DRG_MEM_GC,
	DRG_MEM_VRAM
};


class drgMemBlock;
class drgMemBlockPtr;

class drgMemBlock
{
public:
	drgMemBlock(DRG_MEM_BLOCK_TYPE memtype = DRG_MEM_MAIN);
	~drgMemBlock();

    void Init();
	void CleanUp();
	void Set(unsigned int blocksize, unsigned int smallblocksize);
	void Set(unsigned int blocksize, void* blockptr, unsigned int smallblocksize);
	void Set(unsigned int blocksize, void* blockptr, unsigned int smallblocksize, unsigned int descblocksize, void* descblockptr);
	void* Reserve( unsigned int size, unsigned int align MEM_INFO_DECL );
	void* ReserveZero( unsigned int size, unsigned int align MEM_INFO_DECL );
	void* ReReserve( void* mem, unsigned int size, unsigned int align MEM_INFO_DECL );
	void Release( void* ptr );
	void SmallBlockLock();
	unsigned int MemBlockSize( void* ptr );
	void SetBiggestBlock();
	void PrintInfo(bool full=false, bool csv=false, char* filename=0, char* sReturn=0);
	void PrintGaps( unsigned int minsize=0 );
	void CheckInfo();
	void CheckInfoAt(drgMemBlockPtr* ptr);
	bool Check(drgMemBlockPtr* ptr);
	bool InBlock(void* ptr);
	unsigned int GetBlockNum(drgMemBlockPtr* ptr);
	unsigned int GetTotalBlocks();

	inline bool IsSet()
	{
		return (m_Block!=0);
	}

	inline void* GetHeapPtr()
	{
		return m_Block;
	}

	inline unsigned int GetUsed()
	{
		return m_BlockUsed;
	}

	inline unsigned int GetTotalSize()
	{
		return m_BlockSize;
	}

#if MEM_ENABLE_CHECKS
	inline void EnableCheck(bool enable)
	{
		m_CheckBlocks = enable;
	}

	inline void EnableInBlockCheck(bool enable)
	{
		m_CheckInBlock = enable;
	}
#endif

#if MEM_ENABLE_DEBUG
	static const char*		m_CurDescription;
	static unsigned int		m_CurLineNumber;
	static void SetOwner( const char* desc, unsigned int linenum )
	{
		m_CurDescription = desc;
		m_CurLineNumber = linenum;
	}

	unsigned int	m_AllocInfoFlags;
	unsigned int	m_PrintInfoFlags;
	inline void SetPrintInfo( unsigned int flag )
	{
		m_PrintInfoFlags = flag;
	}
	inline void SetAllocInfo( unsigned int flag )
	{
		m_AllocInfoFlags = flag;
	}
#endif

protected:
	void SetBiggest();
	void SetBestFit( unsigned int size );
	drgMemBlockPtr* FindOpenDescriptor();
	drgMemBlockPtr* FindMemDescriptor(void* ptr);

	drgMemBlockPtr*		m_NullHead;
	drgMemBlockPtr*		m_Current;
	drgMemBlockPtr*		m_Biggest;
	drgMemBlockPtr*		m_Descriptors;
	unsigned char*		m_Block;
	unsigned int		m_BlockSize;
	unsigned int		m_BlockUsed;
	unsigned int		m_DescriptorMax;
	unsigned int		m_DescriptorCur;
	unsigned int		m_HeaderSize;
	bool				m_FreeBlock;
	drgMutex			m_Mutex;
	DRG_MEM_BLOCK_TYPE	m_MemType;
#if MEM_USE_SMALL_BLOCKS
	unsigned int*		m_SmallBlock;
	unsigned int		m_SmallBlockSize;
	unsigned int		m_SmallBlockCount;
	unsigned int		m_SmallBlockPtr;
	unsigned int		m_SmallBlockEnabled;
#endif
#if MEM_ENABLE_CHECKS
	bool				m_CheckBlocks;
	bool				m_CheckInBlock;
#endif

	friend class drgMemBlockPtr;
};


class drgMemBlockPtr
{
public:
	inline void* GetPtr(drgMemBlock* owner) {return &owner->m_Block[m_Mem];};
	inline unsigned int GetSize() {return m_Size;};

protected:
	drgMemBlockPtr();
	~drgMemBlockPtr();

	unsigned int		m_Size;
	unsigned int		m_Mem;
	drgMemBlockPtr*		m_Next;
	drgMemBlockPtr*		m_Prev;

#if MEM_ENABLE_DEBUG
	char				m_DescName[MEM_INFO_LENGTH];
	unsigned int		m_DescNum;
	unsigned int		m_DescFlags;
#endif

	friend class drgMemBlock;
};

extern void* drgMemAllocExt(unsigned int size, const char* desc, unsigned int linenum);
extern void* drgMemAlignExt(unsigned int align, unsigned int size, const char* desc, unsigned int linenum);
extern void* drgMemCAllocExt(unsigned int elements, unsigned int size, const char* desc, unsigned int linenum);
extern void* drgMemReAllocExt(void* mem, unsigned intsize, const char* desc, unsigned int linenum);
extern void drgMemFreeExt(void* mem);


#if defined(_LINUX)
#define DRG_FUNC_MEM_DECL	
#elif defined(_ANDROID)
#define DRG_FUNC_MEM_DECL
#elif defined(_IPHONE)
#define DRG_FUNC_MEM_DECL
#else
#define DRG_FUNC_MEM_DECL	__cdecl
#endif

#if MEM_OVERLOAD_NEW
extern void* DRG_FUNC_MEM_DECL operator new(drgIntSize reportedSize) NEW_POST;
extern void* DRG_FUNC_MEM_DECL operator new[] (drgIntSize reportedSize) NEW_POST;
extern void* DRG_FUNC_MEM_DECL operator new(drgIntSize reportedSize, drgIntSize alignment) NEW_POST;
extern void* DRG_FUNC_MEM_DECL operator new[](drgIntSize reportedSize, drgIntSize alignment) NEW_POST;
extern void DRG_FUNC_MEM_DECL operator delete(void*reportedAddress) DEL_POST;
extern void DRG_FUNC_MEM_DECL operator delete[](void *reportedAddress) DEL_POST;

/*extern void *operator new(drgIntSize reportedSize) NEW_POST;
extern void *operator new[](drgIntSize reportedSize) NEW_POST;
extern void operator delete(void* reportedAddress) DEL_POST;
extern void operator delete[](void* reportedAddress) DEL_POST;*/

#if defined(_3DS)
extern void *operator new(drgIntSize reportedSize, const std::nothrow_t&) DEL_POST;
extern void *operator new[](drgIntSize reportedSize, const std::nothrow_t&) DEL_POST;
extern void operator delete(void* reportedAddress, const std::nothrow_t&) DEL_POST;
extern void operator delete[](void* reportedAddress, const std::nothrow_t&) DEL_POST;
#endif

//inline void *operator new(drgIntSize reportedSize, void* __ptr) DEL_POST { return __ptr; }
//inline void *operator new[](drgIntSize reportedSize, void* __ptr) DEL_POST { return __ptr; }
//inline void operator delete(void* reportedAddress, void* __ptr) DEL_POST { /* nothing */ }
//inline void operator delete[](void* reportedAddress, void* __ptr) DEL_POST { /* nothing */ }

#if !defined(__GNUC__)
// Placment new
void* DRG_FUNC_MEM_DECL operator new (drgIntSize, void * reportedAddress);
void* DRG_FUNC_MEM_DECL operator new[] (drgIntSize, void * reportedAddress);
void DRG_FUNC_MEM_DECL operator delete (void*, void*);
void DRG_FUNC_MEM_DECL operator delete[] (void*, void*);
#define __PLACEMENT_NEW_INLINE
#define __PLACEMENT_VEC_NEW_INLINE
#endif

#endif  // MEM_OVERLOAD_NEW

#if USE_MEM_MANAGER
extern drgMemBlock g_MainBlocks;
#define	malloc(sz)				g_MainBlocks.Reserve(sz, 32 MEM_INFO_AUTO)
#define	calloc(elmts,sz)		g_MainBlocks.ReserveZero(elmts*sz, 32 MEM_INFO_AUTO)
//#define	alloca(elmts,sz)	g_MainBlocks.ReserveZero(sz, 32 MEM_INFO_AUTO)
#define	realloc(ptr,sz)			g_MainBlocks.ReReserve(ptr,sz, 32 MEM_INFO_AUTO)
#define memalign(align,sz)		g_MainBlocks.Reserve(sz, align MEM_INFO_AUTO)
#define	free(ptr)				g_MainBlocks.Release(ptr)
#endif

#endif // __DRG_MEMORY_BLOCK_H__



