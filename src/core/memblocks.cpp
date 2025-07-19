#include "core/memblocks.h"
#include "util/stringn.h"
#include "util/math_def.h"
#include "system/file.h"

#if MEM_ENABLE_DEBUG
const char* g_DefaultNewInfo = "default";
const char* drgMemBlock::m_CurDescription = g_DefaultNewInfo;
unsigned int drgMemBlock::m_CurLineNumber = MEM_INFO_NEW_NUM;
#endif

#if defined(_IPHONE)
#import <Foundation/NSZone.h>
//extern NSAutoreleasePool*		autoreleasepool;
#endif

#if MEM_ENABLE_CHECKS
#define MEM_FOOTER_SIZE			(sizeof(char))
#else
#define MEM_FOOTER_SIZE			0
#endif

#define NEW_ARRAY_HACK_OFFSET	4

#define PTR_IS_ALIGNED(ptr, align) ((((unsigned long long) (ptr)) & ((align) - 1)) == 0)

#undef malloc
#undef free

extern "C" void* DRG_FUNC_MEM_DECL malloc(size_t size);
extern "C" void DRG_FUNC_MEM_DECL free(void* ptr);
void* malloc_zero(unsigned int size)
{
	void* ret = malloc(size);
	memset(ret, 0, size);
	return ret;
}

extern void InitMemory();

drgMemBlock::drgMemBlock(DRG_MEM_BLOCK_TYPE memtype)
{
#ifdef _IPHONE
    //if( !autoreleasepool)
    //    autoreleasepool = [[NSAutoreleasePool alloc] init];
    if( m_Block) return; // The memblocks get initialized earlier on iOS.
#endif
  	m_MemType = memtype;
    Init();
}

void drgMemBlock::Init()
{
    m_NullHead = NULL;
	m_Current = NULL;
	m_Biggest = NULL;
	m_Block = NULL;
	m_BlockSize = 0;
	m_BlockUsed = 0;
	m_FreeBlock = false;
	m_Descriptors = NULL;
	m_DescriptorMax = 0;
	m_DescriptorCur = 0;
	
#if MEM_USE_SMALL_BLOCKS
	m_SmallBlock = NULL;
	m_SmallBlockEnabled = false;
#endif

#if MEM_ENABLE_CHECKS
	m_CheckBlocks = true;
	m_CheckInBlock = false;
#endif
#if MEM_ENABLE_DEBUG
	m_PrintInfoFlags = MEM_INFO_FLAG_BLOCKS | MEM_INFO_FLAG_CHANGES | MEM_INFO_FLAG_CHECK_ONLY;
	m_AllocInfoFlags = 0;
#endif
}

drgMemBlock::~drgMemBlock()
{
	CleanUp();
}

void drgMemBlock::CleanUp()
{
	drgPrintOut( "Cleaning up memory manager\r\n" );
	if(m_FreeBlock)
	{
#if defined(_WII)
		if(m_MemType==DRG_MEM_FAST)
			MEMFreeToExpHeap(g_ExpHeap_Fast, m_Block);
		else
			MEMFreeToExpHeap(g_ExpHeap_Main, m_Block);
#elif defined(_3DS)
		if(m_MemType==DRG_MEM_FAST)
			g_AppDeviceHeap.Free(m_Block);
		else
			g_AppMainHeap.Free(m_Block);
#elif (defined(_IPHONE) || defined(_MAC))
        NSDeallocateMemoryPages( m_Block, m_BlockSize);
#else
		free(m_Block);
#endif
	}
	m_NullHead = NULL;
	m_Current = NULL;
	m_Biggest = NULL;
	m_Block = NULL;
	m_BlockSize = 0;
	m_BlockUsed = 0;
	m_FreeBlock = false;

#if MEM_USE_SMALL_BLOCKS
	m_SmallBlock = NULL;
	m_SmallBlockEnabled = false;
	m_SmallBlockEnabled = false;
#endif

	m_Descriptors = NULL;
	m_DescriptorMax = 0;
	m_DescriptorCur = 0;
}

void drgMemBlock::Set(unsigned int blocksize, void* blockptr, unsigned int smallblocksize)
{
	drgPrintOut("Allocating heap size of %i  (%i)\n", blocksize, smallblocksize);
	drgMutexLock lockmutex(&m_Mutex);
	m_Block = (unsigned char*)blockptr;
	if(m_Block==NULL)
		drgPrintOut("Error initializing memory for manager!!\n");
	m_BlockSize = blocksize;
	m_HeaderSize = (m_Descriptors==NULL)?sizeof(drgMemBlockPtr):0;

	if(m_Descriptors==NULL)
		m_NullHead=(drgMemBlockPtr*)m_Block;
	else
		m_NullHead=FindOpenDescriptor();
	m_NullHead->m_Mem = m_HeaderSize;
	m_NullHead->m_Size = DRG_MAX(128, smallblocksize);
	m_NullHead->m_Next = NULL;
	m_NullHead->m_Prev = NULL;
#if MEM_ENABLE_DEBUG
	strcpy(m_NullHead->m_DescName, "(SMALL BLOCK)");
	m_NullHead->m_DescNum = 0;
	m_NullHead->m_DescFlags = MEM_INFO_FLAG_CHECKED | MEM_INFO_FLAG_PRINTED;
#endif
	m_BlockUsed += m_NullHead->m_Size + m_HeaderSize;
	if(m_MemType!=DRG_MEM_VRAM)
		drgMemory::MemSet(m_NullHead->GetPtr(this), 0, m_NullHead->m_Size);
	m_Current = m_NullHead;
	m_Biggest = m_Current;

#if MEM_USE_SMALL_BLOCKS
	m_SmallBlockEnabled = false;
	if(smallblocksize>0)
	{
		m_SmallBlock = (unsigned int*)m_NullHead->GetPtr(this);
		m_SmallBlockSize = smallblocksize/BLOCK_SMALL_SIZE;
		m_SmallBlockCount = 0;
		m_SmallBlockPtr = 0;
		m_SmallBlockEnabled = true;
	}
#endif
}

void drgMemBlock::Set(unsigned int blocksize, unsigned int smallblocksize)
{
	unsigned char* blockptr = NULL;
#if defined(_WII)
	if(m_MemType==DRG_MEM_FAST)
		blockptr = (unsigned char*)MEMAllocFromExpHeapEx(g_ExpHeap_Fast, blocksize, 128);
	else
		blockptr = (unsigned char*)MEMAllocFromExpHeapEx(g_ExpHeap_Main, blocksize, 128);

#elif defined(_3DS)
	if(m_MemType==DRG_MEM_FAST)
		blockptr = (unsigned char*)g_AppDeviceHeap.Allocate(blocksize);
	else
		blockptr = (unsigned char*)g_AppMainHeap.Allocate(blocksize);
#elif (defined(_IPHONE) || defined(_MAC))
    //NSUInteger i = NSRealMemoryAvailable();
    //drgPrintOut("Free heap space = %d\n", NSRealMemoryAvailable());
    //if( !autoreleasepool)
    //    autoreleasepool = [[NSAutoreleasePool alloc] init];
    if( m_Block) return;
    drgPrintOut("Allocating %d MemBlock space\n", blocksize);
    blockptr = (unsigned char*)NSAllocateMemoryPages (blocksize);
    //drgPrintOut("Free heap space = %d\n", NSRealMemoryAvailable());
#else
	blockptr = (unsigned char*)malloc(blocksize);
    
#if MEM_ENABLE_CHECKS
	drgMemory::MemSet(blockptr, 0xDEDE, blocksize);
#endif
#endif

	m_FreeBlock = true;
	m_Descriptors = NULL;
	m_DescriptorMax = 0;
	m_DescriptorCur = 0;

	Set(blocksize, blockptr, smallblocksize);
}

void drgMemBlock::Set(unsigned int blocksize, void* blockptr, unsigned int smallblocksize, unsigned int descblocksize, void* descblockptr)
{
	m_Descriptors = (drgMemBlockPtr*)descblockptr;
	drgMemory::MemSet(m_Descriptors, 0, descblocksize);
	m_DescriptorMax = descblocksize/sizeof(drgMemBlockPtr);
	m_DescriptorCur = 0;

	Set(blocksize, blockptr, smallblocksize);
}

void drgMemBlock::SmallBlockLock()
{
#if MEM_USE_SMALL_BLOCKS
	m_SmallBlockEnabled = false;
#endif
}

void* drgMemBlock::ReReserve( void* mem, unsigned int size, unsigned int align MEM_INFO_DECL )
{
	if(mem==NULL)
		return Reserve( size, align MEM_INFO_PASS );
	void* retmem;

#if MEM_USE_SMALL_BLOCKS
	// check for small block
	if(((unsigned long long)mem) < (((unsigned long long)m_NullHead->GetPtr(this))+m_NullHead->GetSize()))
	{
		retmem = Reserve( size, align MEM_INFO_PASS );
		m_Mutex.Lock();
		SMALL_BLOCK* curptr = (SMALL_BLOCK*)mem;
		drgMemory::MemCopy(retmem, mem, curptr->size);
		m_Mutex.Unlock();
		Release(mem);
		return retmem;
	}
#endif

	m_Mutex.Lock();
	drgMemBlockPtr* check = FindMemDescriptor(mem);
	unsigned int memsize = check->GetSize();
#if MEM_ENABLE_CHECKS
	memsize -= MEM_FOOTER_SIZE;
#endif
	if (size <= memsize)
	{
		m_BlockUsed -= (memsize - size);
		memsize = size;
		m_Mutex.Unlock();
		return mem;
	}
	m_Mutex.Unlock();

	retmem = Reserve( size, align MEM_INFO_PASS );
	m_Mutex.Lock();
	drgMemory::MemCopy(retmem, mem, memsize);
	m_Mutex.Unlock();
	Release(mem);
	return retmem;
}

void* drgMemBlock::Reserve( unsigned int size, unsigned int align MEM_INFO_DECL )
{
	if(m_Block==NULL)
		InitMemory();

	void* retval = NULL;
	m_Mutex.Lock();
	assert(m_Block);
	assert(m_NullHead);
	assert(size);  
	assert(m_Current);
	assert(m_Biggest);

#if MEM_USE_SMALL_BLOCKS
	// try fitting in small block
	if(m_SmallBlockEnabled && (size<BLOCK_SMALL_SIZE) && (align<=32) && ((m_SmallBlockSize-m_SmallBlockCount)>0))
	{
		unsigned int startptr = m_SmallBlockPtr;
		unsigned int blockcount = startptr;
		SMALL_BLOCK* curptr = (SMALL_BLOCK*)m_SmallBlock;
		do
		{
			if(curptr[blockcount].size==0)
			{
				m_SmallBlockPtr = blockcount+1;
				if(m_SmallBlockPtr>=m_SmallBlockSize)
					m_SmallBlockPtr=0;
				m_SmallBlockCount++;
				curptr[blockcount].size = size;
#if MEM_ENABLE_CHECKS
				if(m_MemType!=DRG_MEM_VRAM)
					drgMemory::MemSet(&curptr[blockcount], 0xBACE, size);
#endif
				retval = (void*)&curptr[blockcount];
				m_Mutex.Unlock();
				return retval;
			}
			blockcount++;
			if(blockcount>=m_SmallBlockSize)
				blockcount = 0;
		} while(blockcount!=startptr);
	}
#endif

	drgMemBlockPtr* retptr;

#if MEM_ENABLE_CHECKS
	size += MEM_FOOTER_SIZE;
#endif

	unsigned int alignpos;
	unsigned int startpos;
	unsigned int totalsize = size + m_HeaderSize;

	if(size>(16*1024))
		SetBestFit(size);
	//else if(size>(4*1024))
	//	m_Current = m_Biggest;
	else if((m_Current==m_Biggest) && (m_Biggest->m_Next!=NULL))
		m_Current = m_Biggest->m_Next;

	retptr = m_Current;

	// iterate till we find an opening
	do
	{
		startpos = (m_Current->m_Mem+m_Current->GetSize());
		if(align>1)
		{
			alignpos = (((unsigned long long)&m_Block[startpos+m_HeaderSize]) % align);
			if(alignpos>0)
				startpos += (align - alignpos);
			assert(PTR_IS_ALIGNED(&m_Block[startpos+m_HeaderSize], align));
		}

		if(m_Current->m_Next)
		{
			unsigned int ptrpos = (m_Current->m_Next->m_Mem-m_HeaderSize);
			if((ptrpos > startpos) && ((ptrpos - startpos) >= totalsize))
			{
				retptr = m_Current;
				if(m_Descriptors==NULL)
					m_Current=(drgMemBlockPtr*)&m_Block[startpos];
				else
					m_Current=FindOpenDescriptor();
				m_Current->m_Mem = (startpos+m_HeaderSize);
				m_Current->m_Size = size;
				m_Current->m_Next = retptr->m_Next;
				if(m_Current->m_Next!=NULL)
					m_Current->m_Next->m_Prev = m_Current;
				m_Current->m_Prev = retptr;
				retptr->m_Next = m_Current;
				m_BlockUsed += totalsize;
#if MEM_ENABLE_CHECKS
				if(m_MemType!=DRG_MEM_VRAM)
					drgMemory::MemSet(m_Current->GetPtr(this), 0xFAFAFAFA, m_Current->GetSize());
#endif
#if MEM_ENABLE_DEBUG
				const char* tmpdsc = "static variable";
				if(desc!=NULL)
					tmpdsc = desc;
				int desclen = strlen(tmpdsc)+1;
				(desclen>=MEM_INFO_LENGTH) ? strcpy(m_Current->m_DescName, &tmpdsc[desclen-MEM_INFO_LENGTH]) : strcpy(m_Current->m_DescName, tmpdsc);
				m_Current->m_DescNum = linenum;
				m_Current->m_DescFlags = ((m_CurLineNumber==linenum)?MEM_INFO_FLAG_NEW:MEM_INFO_FLAG_MALLOC) | m_AllocInfoFlags;
				drgMemBlock::SetOwner( g_DefaultNewInfo, MEM_INFO_NEW_NUM );  // Clear new info
#endif
				retval = m_Current->GetPtr(this);
				m_Mutex.Unlock();
				return retval;
			}
			m_Current = m_Current->m_Next;
		}
		else
		{
			if((m_BlockSize - startpos) > totalsize)
			{
				retptr = m_Current;
				if(m_Descriptors==NULL)
					m_Current=(drgMemBlockPtr*)&m_Block[startpos];
				else
					m_Current=FindOpenDescriptor();
				m_Current->m_Mem = (startpos+m_HeaderSize);
				m_Current->m_Size = size;
				m_Current->m_Next = NULL;
				m_Current->m_Prev = retptr;
				retptr->m_Next = m_Current;
				m_BlockUsed += totalsize;
#if MEM_ENABLE_CHECKS
				if(m_MemType!=DRG_MEM_VRAM)
					drgMemory::MemSet(m_Current->GetPtr(this), 0xFAFAFAFA, m_Current->GetSize());
#endif
#if MEM_ENABLE_DEBUG
				const char* tmpdsc = "static variable";
				if(desc!=NULL)
					tmpdsc = desc;
				int desclen = strlen(tmpdsc)+1;
				(desclen>=MEM_INFO_LENGTH) ? strcpy(m_Current->m_DescName, &tmpdsc[desclen-MEM_INFO_LENGTH]) : strcpy(m_Current->m_DescName, tmpdsc);
				m_Current->m_DescNum = linenum;
				m_Current->m_DescFlags = ((m_CurLineNumber==linenum)?MEM_INFO_FLAG_NEW:MEM_INFO_FLAG_MALLOC) | m_AllocInfoFlags;
				drgMemBlock::SetOwner( g_DefaultNewInfo, MEM_INFO_NEW_NUM );  // Clear new info
#endif
				retval = m_Current->GetPtr(this);
				m_Mutex.Unlock();
				return retval;
			}
			m_Current = m_NullHead;
		}
	} while(m_Current!=retptr);
	m_Mutex.Unlock();

	PrintInfo(false, false, NULL);
	//PrintGaps(1024);
	if( (m_BlockSize - m_BlockUsed) > (totalsize*2) )
		drgPrintError("Alloc Fail For %i Bytes (Fragmented)!!\n", size);
	else
		drgPrintError("Alloc Fail For %i Bytes (Out Of Memory)!!\n", size);
#if MEM_ENABLE_DEBUG
	drgMemBlock::SetOwner( g_DefaultNewInfo, MEM_INFO_NEW_NUM );  // Clear new info
#endif
	return NULL;
}

void* drgMemBlock::ReserveZero( unsigned int size, unsigned int align MEM_INFO_DECL )
{
	assert(m_MemType!=DRG_MEM_VRAM);

	//drgPrintOut("reserve: %i %i\n", size, align);
	void* retmem = Reserve( size, align MEM_INFO_PASS );
	//drgPrintOut("reserve done: %i %i  %p\n", size, align, retmem);
	if(retmem!=NULL)
		drgMemory::MemSet(retmem, 0, size);
	return retmem;
}

void drgMemBlock::Release( void* ptr )
{
	if(ptr==NULL)
		return;
	if(m_Block==NULL)
		return;

	m_Mutex.Lock();
	assert(((unsigned long long)ptr)>=((unsigned long long)m_NullHead->GetPtr(this)));

#if MEM_USE_SMALL_BLOCKS
	// check for small block
	if(((unsigned long long)ptr) < (((unsigned long long)m_NullHead->GetPtr(this))+m_NullHead->GetSize()))
	{
		SMALL_BLOCK* curptr = (SMALL_BLOCK*)ptr;
		if(curptr->size>0)
		{
#if MEM_ENABLE_CHECKS
			if(m_MemType!=DRG_MEM_VRAM)
				drgMemory::MemSet(ptr, 0xFAFA, curptr->size);
#endif
			curptr->size = 0;
			m_SmallBlockCount--;
		}
		m_Mutex.Unlock();
		return;
	}
#endif

	drgMemBlockPtr* check = FindMemDescriptor(ptr);
	m_BlockUsed -= check->GetSize() + m_HeaderSize;
	assert(check!=m_NullHead);


#if MEM_ENABLE_CHECKS
	if (m_CheckInBlock == true)
	{
		drgMemBlockPtr* blockitr = m_NullHead;
		while (blockitr)
		{
			if (blockitr == check)
				break;
			blockitr = blockitr->m_Next;
		}
		if (blockitr == NULL)
			drgPrintOut("**Mem Pointer Not In Memory Block\n");
	}
	if (m_CheckBlocks == true)
	{
		CheckInfoAt(check);
	}
#endif


#if MEM_ENABLE_DEBUG
	if(check->m_DescFlags&MEM_INFO_FLAG_FREE)
		drgPrintError("Memory Already Released!!!\n");
#endif

	if(check->m_Next!=NULL)
	{
		m_Current = m_NullHead;
		check->m_Next->m_Prev = check->m_Prev;
	}

	assert(check->m_Prev);
	m_Current=check->m_Prev;
	check->m_Prev->m_Next = check->m_Next;


	if(m_Biggest==check)
		m_Biggest = m_NullHead;
		
	if(m_Biggest->m_Next)
	{
		if(check->m_Next!=NULL)
		{
			if(((check->m_Next->m_Mem-m_HeaderSize)-(check->m_Prev->m_Mem+check->m_Prev->GetSize())) > ((m_Biggest->m_Next->m_Mem-m_HeaderSize)-(m_Biggest->m_Mem+m_Biggest->GetSize())))
				m_Biggest = check->m_Prev;
		}
		else
		{
			if((m_BlockSize-(check->m_Prev->m_Mem+check->m_Prev->GetSize())) > ((m_Biggest->m_Next->m_Mem-m_HeaderSize)-(m_Biggest->m_Mem+m_Biggest->GetSize())))
				m_Biggest = check->m_Prev;
		}
	}
	else
	{
		if(check->m_Next!=NULL)
		{
			if(((check->m_Next->m_Mem-m_HeaderSize)-(check->m_Prev->m_Mem+check->m_Prev->GetSize())) > (m_BlockSize-(m_Biggest->m_Mem+m_Biggest->GetSize())))
				m_Biggest = check->m_Prev;
		}
		else
		{
			if((m_BlockSize-(check->m_Prev->m_Mem+check->m_Prev->GetSize())) > (m_BlockSize-(m_Biggest->m_Mem+m_Biggest->GetSize())))
				m_Biggest = check->m_Prev;
		}
	}

#if MEM_ENABLE_CHECKS
	if(m_MemType!=DRG_MEM_VRAM)
		drgMemory::MemSet(check->GetPtr(this), 0xEDEDEDED, check->GetSize());
	check->m_Next = NULL;
	check->m_Prev = NULL;
#endif
	check->m_Mem = NULL;
	check->m_Size = 0;

#if MEM_ENABLE_DEBUG
	check->m_DescFlags|=MEM_INFO_FLAG_FREE;
#endif
	m_Mutex.Unlock();
}

unsigned int drgMemBlock::MemBlockSize( void* ptr )
{
	if(ptr==NULL)
		return 0;
	assert(((unsigned long long)ptr)>=((unsigned long long)m_NullHead->GetPtr(this)));
	drgMutexLock lockmutex(&m_Mutex);

#if MEM_USE_SMALL_BLOCKS
	// check for small block
	if(((unsigned long long)ptr) < (((unsigned long long)m_NullHead->GetPtr(this))+m_NullHead->GetSize()))
	{
		SMALL_BLOCK* curptr = (SMALL_BLOCK*)m_SmallBlock;
		return curptr->size;
	}
#endif

	drgMemBlockPtr* check = (drgMemBlockPtr*)ptr;
	check = &check[-1];
	return check->GetSize();
}

void drgMemBlock::SetBiggestBlock()
{
	assert(m_NullHead);
	assert(m_Biggest);
	drgMutexLock lockmutex(&m_Mutex);
	SetBiggest();
}

void drgMemBlock::SetBiggest()
{
	unsigned int biggestgap;
	unsigned int currentgap;
	if(m_Biggest->m_Next==NULL)
		biggestgap = (m_BlockSize-(m_Biggest->m_Mem+m_Biggest->GetSize()));
	else
		biggestgap = ((m_Biggest->m_Next->m_Mem-m_HeaderSize)-(m_Biggest->m_Mem+m_Biggest->GetSize()));

	drgMemBlockPtr* blockitr = m_NullHead->m_Next;
	while(blockitr)
	{
		if( (blockitr->m_Mem-m_HeaderSize)!=(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()) )
		{
			currentgap = ((blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()));
			if(currentgap > biggestgap)
			{
				biggestgap = currentgap;
				m_Biggest = blockitr->m_Prev;
			}
		}
		if(blockitr->m_Next==NULL)
		{
			currentgap = (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize()));
			if(currentgap > biggestgap)
			{
				biggestgap = currentgap;
				m_Biggest = blockitr;
			}
		}
		blockitr = blockitr->m_Next;
	}
}

void drgMemBlock::SetBestFit( unsigned int size )
{
	unsigned int acceptsize = size+32;
	unsigned int bestgap;
	unsigned int currentgap;
	if(m_Current->m_Next==NULL)
		bestgap = (m_BlockSize-(m_Current->m_Mem+m_Current->GetSize()));
	else
		bestgap = ((m_Current->m_Next->m_Mem-m_HeaderSize)-(m_Current->m_Mem+m_Current->GetSize()));

	drgMemBlockPtr* blockitr = m_NullHead->m_Next;
	while(blockitr)
	{
		if( (blockitr->m_Mem-m_HeaderSize)!=(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()) )
		{
			currentgap = ((blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()));
			if((currentgap>size) && (currentgap<bestgap))
			{
				bestgap = currentgap;
				m_Current = blockitr->m_Prev;
				if(bestgap<=acceptsize)
					return;
			}
		}
		if(blockitr->m_Next==NULL)
		{
			currentgap = (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize()));
			if((currentgap>size) && (currentgap<bestgap))
			{
				bestgap = currentgap;
				m_Current = blockitr;
			}
			if(bestgap<=acceptsize)
				return;
		}
		blockitr = blockitr->m_Next;
	}
}

void drgMemBlock::PrintInfo(bool full, bool csv, char* filename, char* sReturn)
{
	if(m_NullHead==NULL)
		return;
	bool usefile = false;
	unsigned int curblock = 1;
	unsigned int printed = 0;
	char linestring[2048];
	drgMemBlockPtr* blockitr = m_NullHead->m_Next;

	drgFile logfile;
	if(filename!=NULL)
	{
		logfile.Open(filename, drgFile::DRG_MODE_WRITE_TEX);
		usefile = logfile.IsGood();
	}

	if(full)
	{
		drgPrintOut("\nMEMORY BLOCK INFO PTR:0x%x SIZE:%u \n", m_Block, m_BlockSize);
		if(csv)
		{
			drgString::PrintFormated(linestring, "BLOCK,PTR,START,END,SIZE,FILE,LINE,TYPE\n");
			if(usefile)
				logfile.Write(linestring, drgString::Length(linestring));
			else
				drgDebug::Print(linestring);
		}
		while(blockitr)
		{
#if MEM_ENABLE_DEBUG
			if(!(m_PrintInfoFlags&MEM_INFO_FLAG_CHECK_ONLY))
			{
				if((!(m_PrintInfoFlags&MEM_INFO_FLAG_CHANGES)) || (!(blockitr->m_DescFlags&MEM_INFO_FLAG_CHECKED)) || ((m_PrintInfoFlags&blockitr->m_DescFlags)&MEM_INFO_FLAG_USER_MASK))
				{
					if(m_PrintInfoFlags&MEM_INFO_FLAG_GAPS)
					{
						if( (blockitr->m_Mem-m_HeaderSize)!=(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()) )
						{
							if(csv)
								drgString::PrintFormated(linestring, "%u,0x%x,%u,%u,%u,%s,%u,%s\n", 0, 0x0, (blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()), (blockitr->m_Mem-m_HeaderSize), ( (blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize())), "GAP", 0, "GAP");
							else
								drgString::PrintFormated(linestring, "**GAP %u\n", ( (blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize())));
							if(usefile)
								logfile.Write(linestring, drgString::Length(linestring));
							else
								drgDebug::Print(linestring);
						}

						if(blockitr->m_Next==NULL)
						{
							if(csv)
								drgString::PrintFormated(linestring, "%u,0x%x,%u,%u,%u,%s,%u,%s\n", 0, 0x0, (blockitr->m_Mem+blockitr->GetSize()), m_BlockSize, (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize())), "GAP", 0, "GAP");
							else
								drgString::PrintFormated(linestring, "**GAP %u\n",  (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize())));
							if(usefile)
								logfile.Write(linestring, drgString::Length(linestring));
							else
								drgDebug::Print(linestring);
						}
					}
					if(m_PrintInfoFlags&MEM_INFO_FLAG_BLOCKS)
					{
						if(csv)
							drgString::PrintFormated(linestring, "%i,0x%x,%u,%u,%u,%s,%u,%s\n", curblock, blockitr->GetPtr(this), blockitr->m_Mem, (blockitr->m_Mem+blockitr->GetSize()), blockitr->GetSize(), blockitr->m_DescName, blockitr->m_DescNum, (blockitr->m_DescFlags&MEM_INFO_FLAG_NEW)?"NEW":"MALLOC");
						else
							drgString::PrintFormated(linestring, "CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitr->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitr->m_Mem, (blockitr->m_Mem+blockitr->GetSize()), blockitr->GetPtr(this), blockitr->GetSize(), blockitr->m_DescName, blockitr->m_DescNum);
						if(usefile)
							logfile.Write(linestring, drgString::Length(linestring));
						else
							drgDebug::Print(linestring);
					}
				}
			}
			blockitr->m_DescFlags |= (MEM_INFO_FLAG_CHECKED|MEM_INFO_FLAG_PRINTED);
#else
			if( (blockitr->m_Mem-m_HeaderSize)!=(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()) )
				drgPrintOut("**GAP %u\n", ( (blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize())));

			if(blockitr->m_Next==NULL)
			{
				drgPrintOut("**GAP %u\n",  (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize())));
			}
			drgPrintOut( "CHUNK:(%i) START:%u END:%u PTR:0x%x SIZE:%u\n", curblock, blockitr->m_Mem, (blockitr->m_Mem+blockitr->GetSize()), blockitr->GetPtr(this), blockitr->GetSize());
#endif
			blockitr = blockitr->m_Next;
			curblock++;
		}
		//drgPrintOut("Printed: %u\n", printed);
	}
	else
	{
		unsigned int biggestgap = 0;
		while(blockitr)
		{
			if( (blockitr->m_Mem-m_HeaderSize)!=(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()) )
				biggestgap = DRG_MAX(( (blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize())), biggestgap);

			if(blockitr->m_Next==NULL)
			{
				biggestgap = DRG_MAX((m_BlockSize-(blockitr->m_Mem+blockitr->GetSize())), biggestgap);
			}

			assert(blockitr != blockitr->m_Next && "Cannot iterate mem blocks: infinite loop");
			blockitr = blockitr->m_Next;
			curblock++;
		}

		if ( !sReturn )
			drgPrintOut("MEMORY BLOCK INFO PTR:0x%x SIZE:%uKB USED:%uKB FREE:%uKB BIGGEST:%uKB COUNT:%u\n", m_Block, m_BlockSize/1024, m_BlockUsed/1024, (m_BlockSize-m_BlockUsed)/1024, biggestgap/1024, curblock);
		else
			drgString::PrintFormated(sReturn, "0x%x;%u KB;%u KB;%u KB;%u KB;%u", m_Block, m_BlockSize/1024, m_BlockUsed/1024, (m_BlockSize-m_BlockUsed)/1024, biggestgap/1024, curblock);
	}

	if(usefile)
		logfile.Close();

#if MEM_ENABLE_DEBUG
	if(m_PrintInfoFlags&MEM_INFO_FLAG_CHECK_ONLY)
		m_PrintInfoFlags &= (~MEM_INFO_FLAG_CHECK_ONLY);
#endif
}

void drgMemBlock::PrintGaps( unsigned int minsize )
{
	unsigned int curblock = 1;
	unsigned int totalgap = 0;
	drgPrintOut("\nMEMORY GAP INFO PTR:0x%x SIZE:%u \n", m_Block, m_BlockSize);
	drgMemBlockPtr* blockitr = m_NullHead;
	while(blockitr)
	{
		if((blockitr->m_Mem-m_HeaderSize)!=(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()))
		{
			totalgap += ((blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize()));
			if(((blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize())) >= minsize)
				drgPrintOut("**GAP %u\n", ((blockitr->m_Mem-m_HeaderSize)-(blockitr->m_Prev->m_Mem+blockitr->m_Prev->GetSize())));
		}

		if(blockitr->m_Next==NULL)
		{
			totalgap += (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize()));
			if( (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize())) >= minsize)
				drgPrintOut("**GAP %u\n",  (m_BlockSize-(blockitr->m_Mem+blockitr->GetSize())));
		}
		blockitr = blockitr->m_Next;
		curblock++;
	}
	drgPrintOut("\nMEMORY TOTAL GAP %u %u OF %u BLOCKS\n", totalgap, (m_BlockSize-m_BlockUsed), curblock);
}

void drgMemBlock::CheckInfoAt(drgMemBlockPtr* ptr)
{
	static int rand_check_all = 1000;
	if (rand_check_all < 0)
	{
		CheckInfo();
		rand_check_all = 10000;
		return;
	}
	rand_check_all--;

	drgMemBlockPtr* blockitr = ptr->m_Prev;
	while (blockitr != ptr->m_Next)
	{
		if (Check(blockitr))
		{
			//#if MEM_ENABLE_DEBUG
			//			drgPrintOut( "PREV_CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitrprev->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitrprev->m_Mem, (blockitrprev->m_Mem+blockitrprev->GetSize()), blockitrprev->GetPtr(this), blockitrprev->GetSize(), blockitrprev->m_DescName, blockitrprev->m_DescNum);
			//			drgPrintOut( "CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitr->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitr->m_Mem, (blockitr->m_Mem+blockitr->GetSize()), blockitr->GetPtr(this), blockitr->GetSize(), blockitr->m_DescName, blockitr->m_DescNum);
			//#endif
			drgPrintWarning("Mem Check Fail!\r\n");
			return;
		}
		blockitr = blockitr->m_Next;
	}
}

void drgMemBlock::CheckInfo()
{
	unsigned int curblock = 1;
	drgMemBlockPtr* blockitrprev = m_NullHead;
	drgMemBlockPtr* blockitr = m_NullHead->m_Next;
	while(blockitr)
	{
		if(blockitr->m_Prev != blockitrprev)
		{
			drgPrintOut("**Mem Pointer Previous Problem\n");
#if MEM_ENABLE_DEBUG
			drgPrintOut( "PREV_CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitrprev->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitrprev->m_Mem, (blockitrprev->m_Mem+blockitrprev->GetSize()), blockitrprev->GetPtr(this), blockitrprev->GetSize(), blockitrprev->m_DescName, blockitrprev->m_DescNum);
			drgPrintOut( "CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitr->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitr->m_Mem, (blockitr->m_Mem+blockitr->GetSize()), blockitr->GetPtr(this), blockitr->GetSize(), blockitr->m_DescName, blockitr->m_DescNum);
#endif
			return;
		}

		if(Check(blockitr))
		{
//#if MEM_ENABLE_DEBUG
//			drgPrintOut( "PREV_CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitrprev->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitrprev->m_Mem, (blockitrprev->m_Mem+blockitrprev->GetSize()), blockitrprev->GetPtr(this), blockitrprev->GetSize(), blockitrprev->m_DescName, blockitrprev->m_DescNum);
//			drgPrintOut( "CHUNK_%c:(%i) START:%u END:%u PTR:0x%x SIZE:%u\t %s(%u)\n", (blockitr->m_DescFlags&MEM_INFO_FLAG_NEW)?'N':'M', curblock, blockitr->m_Mem, (blockitr->m_Mem+blockitr->GetSize()), blockitr->GetPtr(this), blockitr->GetSize(), blockitr->m_DescName, blockitr->m_DescNum);
//#endif
			return;
		}
		blockitrprev = blockitr;
		blockitr = blockitr->m_Next;
		curblock++;
	}

	curblock = 1;
	blockitr = m_Current;
	while(blockitr)
	{
		Check(blockitr);
		blockitr = blockitr->m_Next;
		curblock++;
	}
}

bool drgMemBlock::Check(drgMemBlockPtr* ptr)
{
	if(ptr->m_Mem == 0)
	{
		drgDebug::Notify("**Mem Pointer Null Problem %u\n", (ptr->m_Mem - m_HeaderSize));
		return true;
	}
	if (ptr->m_Size <= MEM_FOOTER_SIZE)
	{
		drgDebug::Notify("**Mem Size Null Problem %u\n", (ptr->m_Mem - m_HeaderSize));
		return true;
	}
	if((((unsigned long long)ptr->GetPtr(this))<((unsigned long long)m_Block)) || (((unsigned long long)ptr->GetPtr(this))>=(((unsigned long long)m_Block)+m_BlockSize)))
	{
		drgDebug::Notify("**Mem Pointer Problem %u\n", (ptr->m_Mem - m_HeaderSize));
		return true;
	}
	if(ptr->m_Prev)
	{
		if((ptr->m_Mem-m_HeaderSize)!=(ptr->m_Prev->m_Mem+ptr->m_Prev->GetSize()))
		{
			if(((ptr->m_Mem-m_HeaderSize)-(ptr->m_Prev->m_Mem+ptr->m_Prev->GetSize()))>=(m_BlockSize-m_BlockUsed))
			{
				drgDebug::Notify("**Mem Gap To Big %u\n", ((ptr->m_Mem - m_HeaderSize) - (ptr->m_Prev->m_Mem + ptr->m_Prev->GetSize())));
				return true;
			}
		}
		if(((unsigned long long)ptr->m_Prev->GetPtr(this))>=((unsigned long long)ptr->GetPtr(this)))
		{
			drgDebug::Notify("**Mem Prev Order Problem %u  (%u >= %u)\n", (ptr->m_Mem - m_HeaderSize, (unsigned long long)ptr->m_Prev->GetPtr(this), (unsigned long long)ptr->GetPtr(this)));
			return true;
		}
	}
	if(ptr->m_Next && ((unsigned long long)ptr->m_Next->GetPtr(this))<=((unsigned long long)ptr->GetPtr(this)))
	{
		drgDebug::Notify("**Mem Next Order Problem %u  (%u <= %u)\n", (ptr->m_Mem - m_HeaderSize, (unsigned long long)ptr->m_Next->GetPtr(this), (unsigned long long)ptr->GetPtr(this)));
		return true;
	}
	unsigned char* footer = (unsigned char*)ptr->GetPtr(this);
	if (footer[ptr->GetSize() - MEM_FOOTER_SIZE] != 0xFA)
	{
		int len_val = ptr->GetSize() - MEM_FOOTER_SIZE;
		unsigned char* footer_end = &footer[ptr->GetSize() - MEM_FOOTER_SIZE];
		drgDebug::Notify("**Mem at 0x%x has been corrupted, check pointer 0x%x\n", &footer[ptr->GetSize() - MEM_FOOTER_SIZE], footer);
		return true;
	}
	return false;
}

bool drgMemBlock::InBlock(void* ptr)
{
	if(ptr == NULL)
		return false;
	drgMemBlockPtr* check = FindMemDescriptor(ptr);
	unsigned int blocktest = GetBlockNum(check);
	if(blocktest==0)
	{
		drgDebug::Notify("**Mem Pointer Not In Block!\n");
		return false;
	}
	return true;
}

unsigned int drgMemBlock::GetTotalBlocks()
{
	unsigned int curblock = 0;
	drgMemBlockPtr* blockitr = m_NullHead;
	while(blockitr)
	{
		blockitr = blockitr->m_Next;
		curblock++;
	}
	return curblock;
}

unsigned int drgMemBlock::GetBlockNum(drgMemBlockPtr* ptr)
{
	unsigned int curblock = 1;
	drgMemBlockPtr* blockitr = m_NullHead;
	while(blockitr)
	{
		if(blockitr==ptr)
			return curblock;
		blockitr = blockitr->m_Next;
		curblock++;
	}

	curblock = 1;
	blockitr = m_Current;
	while(blockitr)
	{
		if(blockitr==ptr)
			return curblock;
		blockitr = blockitr->m_Next;
		curblock++;
	}
	return 0;
}

drgMemBlockPtr* drgMemBlock::FindOpenDescriptor()
{
	unsigned int start = m_DescriptorCur;
	do
	{
		m_DescriptorCur = ((m_DescriptorCur+1)%m_DescriptorMax);
		if(m_Descriptors[m_DescriptorCur].GetSize() == 0)
			return &m_Descriptors[m_DescriptorCur];
	} while(m_DescriptorCur!=start);
	drgPrintError("Failed to find Open Mem Descriptor!!\n");
	return NULL;
}


drgMemBlockPtr* drgMemBlock::FindMemDescriptor(void* ptr)
{
	if(m_Descriptors==NULL)
		return &((drgMemBlockPtr*)ptr)[-1];

	unsigned int checkval = (((drgUInt64)ptr)-((drgUInt64) m_Block));
	for(unsigned int count=0; count<m_DescriptorMax; ++count)
	{
		if(m_Descriptors[count].m_Mem == checkval)
			return &m_Descriptors[count];
	}
	drgPrintError("Failed to find Mem Descriptor!!\n");
	return NULL;
}

drgMemBlockPtr::drgMemBlockPtr()
{
	m_Mem = 0;
	m_Size = 0;
	m_Next = NULL;
	m_Prev = NULL;
}

drgMemBlockPtr::~drgMemBlockPtr()
{
}



void* drgMemAllocExt(unsigned int size, const char* desc, unsigned int linenum)
{
	return drgMemAlloc(size);
}

void* drgMemAlignExt(unsigned int align, unsigned int size, const char* desc, unsigned int linenum)
{
	return drgMemAlign(align, size);
}

void* drgMemCAllocExt(unsigned int elements, unsigned int size, const char* desc, unsigned int linenum)
{
	return drgMemAllocZero(elements*size);
}

void* drgMemReAllocExt(void* mem, unsigned intsize, const char* desc, unsigned int linenum)
{
	return drgMemReAlloc(mem, intsize);
}

void drgMemFreeExt(void* mem)
{
	drgMemFree(mem);
}


#if MEM_OVERLOAD_NEW
void* DRG_FUNC_MEM_DECL operator new(drgIntSize reportedSize)
{
	if(reportedSize==0)
		return NULL;
	//if(drgMemBlock::m_CurLineNumber==MEM_INFO_NEW_NUM)
	//	drgMemBlock::m_CurLineNumber = MEM_INFO_NEW_NUM;
	void* ret = NULL;
	ret = drgMemAlloc(reportedSize);
	//ret = drgMemAllocZero(reportedSize);
	return ret;
}

void* DRG_FUNC_MEM_DECL operator new[](drgIntSize reportedSize)
{
    if(reportedSize==0)
		return NULL;
#if !(defined(_IPHONE))
	drgPrintError("If you really need this, contact David or Dylan\r\n");
	char* retval = (char*)drgMemAlloc(reportedSize + NEW_ARRAY_HACK_OFFSET);
	retval[0] = (char)0xFEEF;
	return &retval[NEW_ARRAY_HACK_OFFSET];
#else
    //drgPrintError("new[]\r\n");
    return drgMemAllocZero(reportedSize);
#endif
}

void DRG_FUNC_MEM_DECL operator delete(void* reportedAddress)
{
	drgMemFree(reportedAddress);
}

void DRG_FUNC_MEM_DECL operator delete[](void* reportedAddress)
{
#if !(defined(_IPHONE))
	drgPrintError("If you really need this, contact David or Dylan\r\n");
	char* del_ptr = ((char*)reportedAddress)-NEW_ARRAY_HACK_OFFSET;
	drgMemFree(del_ptr);
#else
    //drgPrintError("delete[]\r\n");
    char* del_ptr = ((char*)reportedAddress);
	drgMemFree(del_ptr);
#endif
}


#endif


#if MEM_OVERLOAD_NEW
#if !defined(_IPHONE) && !defined(__GNUC__)

void* operator new (drgIntSize, void * reportedAddress)
{
	return reportedAddress;
}

void* operator new[] (drgIntSize, void * reportedAddress)
{
	return reportedAddress;
}

void operator delete (void *, void *)
{
}

void operator delete[] (void *, void *)
{
}
#endif
#endif


#include "util/drg_gc.h"
void drgGCInitExt(unsigned int size, notify_func* func, bool debug)
{
	drgGarbageColector::GetDefault()->Init(size, func, debug);
}

void drgGCDeInitExt()
{
	drgGarbageColector::GetDefault()->DeInit();
}

void* drgGCAllocExt(unsigned int size, unsigned int frame, bool refs)
{
	return drgGarbageColector::GetDefault()->Alloc(size, frame, refs);
}

void drgGCFreeExt(void* ptr)
{
	drgGarbageColector::GetDefault()->Free(ptr);
}

bool drgGCIdleCountExt(unsigned int frame, int count)
{
	return drgGarbageColector::GetDefault()->IdleCount(frame, count);
}

int drgGCCollectNowExt(unsigned int frame)
{
	return drgGarbageColector::GetDefault()->CollectNow(frame);
}

void drgEnableExt()
{
	drgGarbageColector::GetDefault()->Enable();
}

bool drgDisableExt( bool block )
{
	return drgGarbageColector::GetDefault()->Disable(block);
}

void drgGCRootExt(void* ptr)
{
	drgGarbageColector::GetDefault()->Root(ptr);
}

void drgGCUnRootExt(void* ptr)
{
	drgGarbageColector::GetDefault()->UnRoot(ptr);
}

int drgGCRegTypeExt(char* desc, int flags, clear_func clrf, finalize_func* finalf)
{
	return drgGarbageColector::GetDefault()->RegType(desc, flags, clrf, finalf);
}

bool drgGCSetTypeExt(void* ptr, int type)
{
	return drgGarbageColector::GetDefault()->SetType(ptr, type);
}

void drgGCSetDataExt(void* ptr, void* data)
{
	return drgGarbageColector::GetDefault()->SetData(ptr, data);
}

unsigned int drgGCUsedAmountExt()
{
	return drgGarbageColector::GetDefault()->UsedAmount();
}

clear_func* drgGetClearExt(int type)
{
	return drgGarbageColector::GetDefault()->GetClear(type);
}

void drgPrintInfoExt()
{
	drgGarbageColector::GetDefault()->PrintInfo();
}



