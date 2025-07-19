
#include <errno.h>
#include "util/drg_gc.h"
#include "system/debug.h"


//#include <time.h>
//#include <windows.h>
//bool mm_debug_enabled=true;
//class testtime
//{
//public:
//	testtime(char* charstr)
//	{
//		if(mm_debug_enabled)
//		{
//			dbgval = charstr;
//			tick = GetTickCount();
//		}
//	}
//	~testtime()
//	{
//		if(mm_debug_enabled)
//		{
//			tick = (GetTickCount() - tick);
//			if(tick>0)
//				drgPrintOut("time for %s %d\n", dbgval, tick);
//		}
//	}
//
//	char* dbgval;
//	DWORD tick;
//};



#define PAGEBITS        12
//#define BLOCKBITS       (PAGEBITS+1)
#define BLOCKBITS       PAGEBITS
#define BLOCKSIZE       (1<<BLOCKBITS)

#define EXTRA_PTRSIZE	(sizeof(void*))
#define MIN_TYPES		0x100
#define MIN_MANAGED		(2 * 1024 * 1024)
#define MIN_ROOTS		(128  * 1024)
#define MIN_STACK		(128  * 1024)


#define BITL               1  // LIVE/OBSOLETE address
#define BITN               2  // NOTIFY
#define BITW               4  // wait collect
#define BITM               8

#define LIVE(p)            (((gc_ptr)(p)) & BITL)
#define NOTIFY(p)          (((gc_ptr)(p)) & BITN)
#define WAIT(p)            (((gc_ptr)(p)) & BITW)
#define OBSOLETE           LIVE

#define MARK_LIVE(p)       { p = (void *)((gc_ptr)(p) | BITL); }
#define MARK_NOTIFY(p)     { p = (void *)((gc_ptr)(p) | BITN); }
#define MARK_WAIT(p)       { p = (void *)((gc_ptr)(p) | BITW); }
#define MARK_OBSOLETE      MARK_LIVE

#define UNMARK_LIVE(p)     { p = (void *)((gc_ptr)(p) & ~BITL); }
#define UNMARK_NOTIFY(p)   { p = (void *)((gc_ptr)(p) & ~BITN); }
#define UNMARK_WAIT(p)     { p = (void *)((gc_ptr)(p) & ~BITW); }


#define gc_debug(...)      m_DebugEnabled ? (\
	drgPrintOut("mm(%s): ", __FUNCTION__), \
	drgPrintOut(__VA_ARGS__), 0) : 1



static void def_clear_ptr(void* ptr, unsigned int size)
{
	drgMemory::MemSet(ptr, 0, size);
}

/*#if defined(_IPHONE)
    drgGarbageColector* drgGarbageColector::m_pInstance = NULL;
#else*/
    drgGarbageColector drgGarbageColector::m_Instance;
//#endif

drgGarbageColector::drgGarbageColector() : m_GCSpace(DRG_MEM_GC)
{
	m_TotalMalloc = 0;
	m_CollectAllocs = 0;
	m_NumCollects = 0;
	m_CollectStage = 0;
	m_VolAllocs = 0;
	m_CollectMin = 1000;
	m_StackLast = -1;
	m_RootsLast = -1;
	m_TypesLast = -1;
	m_ManLast = -1;
	m_MarkInProgress = false;
	m_ManIsCompact = true;
	m_GCdisabled = false;
	m_CollectInProgress = false;
	m_CollectRestart = false;
	m_StackOverflowed = false;
	m_StackOverflowed2 = false;
}

drgGarbageColector::~drgGarbageColector()
{
}

void drgGarbageColector::Init(unsigned int size, notify_func* notifyfunc, bool debug)
{
	assert(sizeof(gc_info) <= GC_MIN_HUNKSIZE);

	m_ClientNotify = notifyfunc;

	m_DebugEnabled = debug;

	if (m_GCSpace.IsSet())
	{
		drgDebug::Notify("gc is already initialized\n");
		return;
	}

	void* mallocspace=drgMemAlloc(size);
	m_GCSpace.Set(size, mallocspace, 0);
	m_HeapStart = (drgIntAddr)mallocspace;
	m_HeapEnd = (m_HeapStart+size);

	m_VolumeThreshold = size/20;
	if(m_VolumeThreshold > GC_MAX_VOLUME)
		m_VolumeThreshold = GC_MAX_VOLUME;
	gc_debug("threshold : %6Id KByte\n", m_VolumeThreshold/(1<<10));

	// set up type directory
	m_Types = (gc_typerec*)AllocInternal((MIN_TYPES * sizeof(gc_typerec)));
	assert(m_Types);
	m_TypesSize = MIN_TYPES;
	{
		int mtype;
		mtype = RegType("blob", GC_MARK_NONE, def_clear_ptr, 0);
		assert(mtype == DRG_GC_BLOB);
		mtype = RegType("refs", GC_MARK_REFS, def_clear_ptr, 0);
		assert(mtype == DRG_GC_REFRENCE);
	}
	assert(m_TypesLast == DRG_GC_REFRENCE);

	/* set up other bookkeeping structures */
	m_Managed = (void**)AllocInternal((MIN_MANAGED * sizeof(void *)));
	assert(m_Managed);
	m_ManSize = MIN_MANAGED;

	m_RootList = (const void**)AllocInternal((MIN_ROOTS * sizeof(void *)));
	assert(m_RootList);
	m_RootsSize = MIN_ROOTS;

	m_MarkingStack = (const void**)AllocInternal((MIN_STACK * sizeof(void *)));
	m_MarkingStackReset=false;
	m_StackSize = MIN_STACK;
	m_StackOverflowed = false;

	gc_debug("gc init done\n");
}

void drgGarbageColector::DeInit()
{
	m_GCSpace.CleanUp();
}

void* drgGarbageColector::Alloc(unsigned int size, unsigned int frame, bool refs)
{
	if(size % GC_MIN_HUNKSIZE)
		size = ((size>>GC_ALIGN_NUM_BITS)+1)<<GC_ALIGN_NUM_BITS;

	DRG_GC_TYPE type = refs ? DRG_GC_REFRENCE : DRG_GC_BLOB;
	void* ptr = NULL;

	// make sure s is a multiple of MIN_HUNKSIZE
	assert(size%GC_MIN_HUNKSIZE == 0);

//	if (size > GC_SIZE_MAX)
//	{
//		drgDebug::Notify("size exceeds MM_SIZE_MAX\n");
//		return NULL;
//	}

	int extrasize = GC_MIN_HUNKSIZE;  // + space for info

	ptr = AllocInternal(size + extrasize);
	assert(!GC_LBITS(ptr));

	if (ptr)
	{
		gc_info *info = (gc_info*)(ptr);
		assert((size/GC_MIN_HUNKSIZE) < 65534);
		info->type = type;
		info->numhnk = size/GC_MIN_HUNKSIZE;
		m_TotalMalloc += info->numhnk;
		info->frame = frame;
		info->data = NULL;
		MaybeTriggerCollect(size, frame);

		ptr = Seal((const char*)(ptr));
	}
	else
	{
		m_CollectRequested = true;
	}

	if (!ptr)
	{
		drgDebug::Notify("GC Allocation Failed!!\n");
		return NULL;
	}
	if (m_Types[type].clear)
		m_Types[type].clear(ptr, size);
	Manage(ptr, type);
	return ptr;
}

void drgGarbageColector::Free(void* ptr)
{
	int i = FindManaged(ptr);
	if(i >= 0)
		ReclaimOffheap(i);
}

void drgGarbageColector::ReclaimOffheap(int idx)
{
	if(OBSOLETE(m_Managed[idx]))
	{
		drgPrintOut("GC OBSOLETE %i!!\n", idx);
		return;
	}

	void* qptr = GC_CLRPTR(m_Managed[idx]);
	gc_info* info_q = (gc_info*)(UnSeal((const char*)(qptr)));
	m_TotalMalloc -= info_q->numhnk;
	finalize_func* func = m_Types[info_q->type].finalize;
	if(func)
	{
		Disable(false);
		errno = 0;
		func(qptr, info_q->data);
		if (errno)
		{
			char *errmsg = strerror(errno);
			gc_debug("finalizer of object 0x%Ix (%s) caused an error:\n%s\n", GC_PPTR(qptr), m_Types[info_q->type].name, errmsg);
		}
		Enable();
	}
	qptr = info_q;

	if (NOTIFY(m_Managed[idx]))
	{
		UNMARK_NOTIFY(m_Managed[idx]);
		m_ClientNotify(m_Managed[idx]);
	}

	MARK_OBSOLETE(m_Managed[idx]);
	m_ManIsCompact = false;

	m_GCSpace.Release(qptr);
}

int drgGarbageColector::RegType(const char* desc, int flags, clear_func clrf, finalize_func* finalf)
{
	if (!desc || !desc[0])
	{
		drgDebug::Notify("first argument invalid\n");
		return DRG_GC_UNDEFINED;
	}

	// check if type is already registered
	for (int t = 0; t <= m_TypesLast; t++)
	{
		if (0==strcmp(m_Types[t].name, desc) && (clrf == m_Types[t].clear) && (finalf == m_Types[t].finalize) && (flags == m_Types[t].flags))
		{
			drgDebug::Notify("attempt to re-register memory type\n");
			return t;
		}
	}

	/* register new memtype */
	m_TypesLast++;
	if (m_TypesLast == m_TypesSize)
	{
		gc_debug("enlarging type directory\n");
		m_TypesSize *= 2;
		m_Types = NULL;
		assert(m_Types);
	}
	assert(m_TypesLast < m_TypesSize);

	gc_typerec *rec = &(m_Types[m_TypesLast]);

	int stringlength = strlen(desc) + 1;
	char* newString = NULL;
	newString = (char*)AllocInternal((sizeof(char) * stringlength));
	drgMemory::MemCopy(newString, (void*)desc, stringlength);
	rec->name = newString;
	rec->flags = flags;
	rec->clear = clrf;
	rec->finalize = finalf;
	return m_TypesLast;
}

void drgGarbageColector::Manage(const void* ptr, int type)
{
	assert(GC_ADDRESS_VALID(ptr));

	m_ManLast++;
	if (m_ManLast == m_ManSize)
	{
		m_ManSize *= 2;
		drgDebug::Notify("enlarging managed table to %d\n", m_ManSize);
		m_Managed = NULL;
		assert(m_Managed);
	}
	assert(m_ManLast < (unsigned int)m_ManSize);
	m_Managed[m_ManLast] = (void*)ptr;
	GC_INFO_I(ptr) = m_ManLast;
	MARK_LIVE(ptr);
}

bool drgGarbageColector::IdleCount(unsigned int frame, int count)
{
	if(count<=0)
		return false;
	static int count_val = 0;
	count_val++;
	if (count_val < count)
		return false;
	count_val = 0;
	// create some work for ourselves
	CollectNow(frame);

	return true;
}

void drgGarbageColector::MaybeTriggerCollect(int size, unsigned int frame)
{//return;
	m_VolAllocs += size;
	if (m_GCdisabled || m_CollectInProgress)
		return;

	if ((m_VolAllocs >= m_VolumeThreshold) || m_CollectRequested)
	{
		CollectNow(frame);
		m_VolAllocs = 0;
	}
}

int drgGarbageColector::CollectNow(unsigned int frame)
{
	if (m_CollectInProgress)
		return 0;
	if (m_GCdisabled)
	{
		m_CollectRequested = true;
		return 0;
	}

	if((m_CollectFrame>frame) || (m_CollectRestart==true))
	{
		m_CollectStage = 0;
		m_CollectInProgress = false;
		m_CollectRestart = false;
	}

	int collectcount = 0;

	switch(m_CollectStage)
	{
	case 0:
		m_StackLast = -1;
		m_CollectAllocs = 0;
		m_CollectFrame = frame;
		if(m_NumAllocs > m_CollectMin)
			m_CollectAllocs = m_NumAllocs-m_CollectMin;
		CollectPrologue();

		// Trace live objects from root objects
		for (int cur_root=0; cur_root <= m_RootsLast; ++cur_root)
		{
			if (m_RootList[cur_root])
			{
				if(!IsManaged(m_RootList[cur_root]))
				{
					drgDebug::Notify("root at 0x%Ix is not a managed address\n", GC_PPTR(m_RootList[cur_root]));
				}
				else if (m_RootList[cur_root])
				{
					MarkStackPush(m_RootList[cur_root]);
				}
			}
		}
		++m_CollectStage;

	case 1:
		TraceFromStack();
		++m_CollectStage;

	case 2:
		Mark();
		collectcount = Sweep();
		CollectEpilogue();
		m_CollectStage=0;
		//m_GCSpace.PrintInfo();
	}

	return collectcount;
}

void drgGarbageColector::TraceFromStack()
{
process_stack:
	while(!MarkStackEmpty())
	{
		const void* ptr = m_MarkingObject = MarkStackPop();
		int type = m_MarkingType = GetType(ptr);
		if (m_Types[type].flags & GC_MARK_REFS)
			MarkRefs((const void**)ptr);
	}
	if (m_StackOverflowed)
	{
		MarkStackRecover();
		goto process_stack;
	}
}

bool drgGarbageColector::Mark()
{
	m_MarkInProgress = true;

	int countto = m_ManLast;
	int countfrom=0;

	for (; countfrom <= countto; ++countfrom)
	{
		int type = GC_INFO_T(m_Managed[countfrom]);
		finalize_func *finalizef = m_Types[type].finalize;
		if (!LIVE(m_Managed[countfrom]) && finalizef)
		{
			if(m_Types[type].flags & GC_MARK_REFS)
				MarkRefs((const void**)GC_CLRPTR(m_Managed[countfrom]));
			TraceFromStack();
			UNMARK_LIVE(m_Managed[countfrom]); // break cycles
		}
	}

	if(countfrom <= m_ManLast)
	{
		m_MarkInProgress = false;
		return false;
	}

	m_MarkInProgress = false;
	return true;
}

int drgGarbageColector::Sweep()
{
	int sweepcount = 0;
	int countfrom = 0;
	int countto = m_ManLast;

	TraceFromStack();
	for (; countfrom <= countto; ++countfrom)
	{
		if (LIVE(m_Managed[countfrom]))
		{
			UNMARK_LIVE(m_Managed[countfrom]);
		}
		else if(GC_INFO_F(m_Managed[countfrom]) < m_CollectFrame)
		{
			ReclaimOffheap(countfrom);
			++sweepcount;
		}
	}

	if(m_DebugEnabled)
	{
		gc_debug("%d objects reclaimed\n", sweepcount);
	}

	return sweepcount;
}

void drgGarbageColector::CollectPrologue()
{
	assert(!m_CollectInProgress);
	assert(!m_StackOverflowed2);

	m_CollectInProgress = true;
	gc_debug("%dth collect after %d allocations:\n", m_NumCollects, m_NumAllocs);
	gc_debug("mean alloc %.2f bytes)\n", (m_NumAllocs ? ((double)m_VolAllocs)/m_NumAllocs : 0));
}

void drgGarbageColector::CollectEpilogue()
{
	assert(m_CollectInProgress);

	if (m_StackOverflowed2)
	{
		// only effective with synchronous collect
		m_StackSize *= 2;
		gc_debug("enlarging marking stack to %d\n", m_StackSize);
		m_StackOverflowed2 = false;
	}

	m_MarkingType = DRG_GC_UNDEFINED;
	m_MarkingObject = NULL;
	m_CollectRequested = false;
	m_CollectInProgress = false;
	m_NumCollects += 1;
	CompactManaged();
}

void drgGarbageColector::CompactManaged()
{
	assert(!m_CollectInProgress);

	if(m_ManIsCompact)
		return;

	int i, n = 0;
	for (i = 0; i <= m_ManLast; i++)
	{
		if (!OBSOLETE(m_Managed[i]))
		{
			GC_INFO_I(m_Managed[i]) = n;
			m_Managed[n++] = m_Managed[i];
		}
	}
	m_ManLast = n-1;

	// shrink when much bigger than necessary
	if ((unsigned int)m_ManLast*4<m_ManSize && m_ManSize > MIN_MANAGED)
	{
		m_ManSize /= 2;
		m_Managed = NULL;//static_cast<void **__restrict>(realloc(m_Managed, m_ManSize*sizeof(void *)));
		assert(m_Managed);
	}
	m_ManIsCompact = true;
}

void drgGarbageColector::CheckManaged()
{
	if(!m_DebugEnabled)
		return;
	// do linear search on excess part
	for (int i = 0; i <= m_ManLast; ++i)
	{
		if(GC_INFO_I(m_Managed[i]) != i)
			gc_debug("Managed list is garbage! %d : %d\n", GC_INFO_I(m_Managed[i]), i);
	}
}

bool drgGarbageColector::Disable(bool block)
{
	// block when a collect is in progress
	if (block && m_CollectInProgress)
		drgDebug::Error("Cant block GC while in collect!!");

	bool nogc = m_GCdisabled;
	m_GCdisabled = true;
	return nogc;
}

void drgGarbageColector::Enable()
{
	m_GCdisabled = false;
}

unsigned int drgGarbageColector::UsedAmount()
{
	return (m_TotalMalloc*GC_MIN_HUNKSIZE)/8;
}

void drgGarbageColector::PrintInfo()
{
	m_GCSpace.PrintInfo();
}

void drgGarbageColector::SetData(void* ptr, void* data)
{
	assert(GC_ADDRESS_VALID(ptr));
	int i = FindManaged(ptr);
	if(i<0)
		return;

	gc_info* info = (gc_info*)(UnSeal((char *)ptr));
	info->data = data;
}

bool drgGarbageColector::SetType(void* ptr, int type)
{
	int i = FindManaged(ptr);
	if(i<0)
		return false;

	int fromtype = GC_INFO_T(m_Managed[i]);
	if(fromtype == type)
		return true;

	if( (m_Types[fromtype].flags==m_Types[type].flags) )
	{
		GC_INFO_T(m_Managed[i]) = type;
		return true;
	}
	return false;
}

void drgGarbageColector::Root(void* ptr)
{
	// avoid creating duplicates
	for (int i = m_RootsLast; i >= 0 ; i--)
	{
		if (m_RootList[i] == ptr)
		{
			gc_debug("attempt to add existing root (ignored)\n");
			return;
		}
	}
	if (ptr && !IsManaged(ptr))
	{
		drgDebug::Notify("root does not contain a managed address\n0x%Ix\n", GC_PPTR(ptr));
		return;
	}

	m_RootsLast++;
	if (m_RootsLast == m_RootsSize)
	{
		m_RootsSize *= 2;
		gc_debug("enlarging root table to %d\n", m_RootsSize);
		m_RootList = NULL;
		assert(m_RootList);
	}

	assert(m_RootsLast < m_RootsSize);
	gc_debug("new root: 0x%Ix\n", GC_PPTR(ptr));
	m_RootList[m_RootsLast] = ptr;
	m_CollectRestart = true;
}

void drgGarbageColector::UnRoot(void* ptr)
{
	assert(m_RootsLast>=0);
	if (m_RootList[m_RootsLast] == ptr)
	{
		m_RootsLast--;
		return;
	}

	bool r_found = false;
	for (int i = 0; i < m_RootsLast; i++)
	{
		if (m_RootList[i] == ptr)
			r_found = true;
		if (r_found)
			m_RootList[i] = m_RootList[i+1];
	}

	if (r_found)
		m_RootsLast--;
	else
		drgDebug::Notify("attempt to unroot non-existing root\n");
	m_CollectRestart = true;
}

void drgGarbageColector::MarkRefs(const void** ptr)
{
	int mgidx;
	int n = GetSize(ptr)/sizeof(void *);
	for (int i = 0; i < n; i++)
	{
		if ( ptr[i] )
		{
			mgidx = FindManagedInMem(ptr[i]);
			if(mgidx>=0)
				if(!LIVE(m_Managed[mgidx]))
					MarkStackPush(GC_CLRPTR(m_Managed[mgidx]));
		}
	}
}

void drgGarbageColector::MarkStackRecover()
{
	if (!m_StackOverflowed)
		return;
	gc_debug("marking stack overflowed, recovering\n");
	assert(MarkStackEmpty());
	m_StackOverflowed = false;
	m_StackOverflowed2 = true;

	for (int i = 0; i <= m_ManLast; i++)
	{
		if (LIVE(m_Managed[i]))
		{
			int type = GC_INFO_T(m_Managed[i]);
			if (m_Types[type].flags & GC_MARK_REFS)
				MarkRefs((const void**)GC_CLRPTR(m_Managed[i]));
		}
	}
}

bool drgGarbageColector::Live(const void *ptr)
{
	int i = FindManaged(ptr);
	assert(i != -1);
	return LIVE(m_Managed[i]);
}

void drgGarbageColector::MarkLive(const void *ptr)
{
	int i = FindManaged(ptr);
	assert(i != -1);
	MARK_LIVE(m_Managed[i]);
}

clear_func* drgGarbageColector::GetClear(int type)
{
	assert((type>=DRG_GC_BLOB) && (type<=m_TypesLast));
	return m_Types[type].clear;
}

finalize_func* drgGarbageColector::GetFinalize(int type)
{
	assert((type>DRG_GC_BLOB) && (type<=m_TypesLast));
	return m_Types[type].finalize;
}





