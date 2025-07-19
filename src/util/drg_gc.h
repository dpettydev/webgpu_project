
#ifndef __DRG_GCOLLECT_H__
#define __DRG_GCOLLECT_H__

#include "core/memmgr.h"

#define GC_PPTR(ptr)		((gc_ptr)(ptr))

#if defined (USE_64_BIT)
	#define GC_ALIGN_NUM_BITS	8
#else
	#define GC_ALIGN_NUM_BITS	4
#endif

#define GC_SIZE_MAX			(4294967295*GC_MIN_HUNKSIZE)
#define GC_MAX_VOLUME		0x300000    // max volume threshold
#define GC_MIN_HUNKSIZE		(1<<GC_ALIGN_NUM_BITS)
#define GC_LBITS(p)			((gc_ptr)(p) & (GC_MIN_HUNKSIZE-1))
#define GC_CLRPTR(p)		((void *)((((gc_ptr)(p)) & ~(GC_MIN_HUNKSIZE-1))))
#define GC_ADDRESS_VALID(p)	(!GC_LBITS(p))
#define GC_TYPE_VALID(t)	((t)>=0 && (t)<=types_last)
#define GC_INDEX_VALID(i)	((i)>=0 && (i)<=man_last)

#define GC_INFO_S(p)		(((gc_info *)(UnSeal((char *)p)))->numhnk*GC_MIN_HUNKSIZE)
#define GC_INFO_T(p)		(((gc_info *)(UnSeal((char *)p)))->type)
#define GC_INFO_F(p)		(((gc_info *)(UnSeal((char *)p)))->frame)
#define GC_INFO_D(p)		(((gc_info *)(UnSeal((char *)p)))->data)
#define GC_INFO_I(p)		(((gc_info *)(UnSeal((char *)p)))->idx)

#define GC_MARK_NONE		(1<<0)
#define GC_MARK_REFS		(1<<1)

#if defined (USE_64_BIT)
typedef unsigned long long gc_ptr;
#else
typedef unsigned int gc_ptr;
#endif

class drgGarbageColector;
typedef void clear_func(void* ptr, unsigned int size);
typedef void finalize_func(void* ptr, void* data);
typedef void notify_func(void* data);

class drgGarbageColector
{
public:
	enum DRG_GC_TYPE
	{
		DRG_GC_UNDEFINED=-1,
		DRG_GC_BLOB=0,
		DRG_GC_REFRENCE
	};

	static inline drgGarbageColector* GetDefault()
	{
/*#if defined(_IPHONE)
        if( !m_pInstance)
            m_pInstance = new drgGarbageColector();
		return m_pInstance;
#else*/
		return &m_Instance;
//#endif
	}
	clear_func* GetClear(int type);
	finalize_func* GetFinalize(int type);

	drgGarbageColector();
	~drgGarbageColector();

	void Init(unsigned int size, notify_func* func, bool debug);
	void DeInit();

	void Enable();
	bool Disable( bool block );

	int RegType(const char* desc, int flags, clear_func clrf, finalize_func* finalf);

	void* Alloc(unsigned int size, unsigned int frame, bool refs);
	void Free(void* ptr);

	bool IdleCount(unsigned int frame, int count);
	int CollectNow(unsigned int frame);

	void Root(void* ptr);
	void UnRoot(void* ptr);
	bool SetType(void* ptr, int type);
	void SetData(void* ptr, void* data);
	bool Live(const void *ptr);
	void MarkLive(const void *ptr);
	unsigned int UsedAmount();
	void PrintInfo();

	inline void CheckInfo();
	inline unsigned int GetSize(const void *ptr);
	inline int GetType(const void *ptr);
	inline bool IsManaged(const void *ptr);

private:

	struct gc_info
	{
		short					type;		// type of memory object 
		unsigned short			numhnk;		// size of memory object in multiples of MIN_HUNKSIZE
		unsigned int			idx;
		unsigned int			frame;
		void*					data;
	};
	
	struct gc_typerec
	{
		char*			name;
		clear_func*		clear; 
		finalize_func*	finalize;
		int             flags;
	};

	void Manage(const void* ptr, int type);
	void ReclaimOffheap(int idx);
	void CompactManaged();
	void CheckManaged();
	void TraceFromStack();
	void MaybeTriggerCollect(int size, unsigned int frame);
	bool Mark();
	void MarkRefs(const void** ptr);
	int Sweep();
	void CollectPrologue();
	void CollectEpilogue();

	void MarkStackRecover();

	inline void* AllocInternal(unsigned int size);
	inline void* Seal(const char *ptr);
	inline void* UnSeal(const char *ptr);
	inline int FindManaged(const void *ptr);
	inline int FindManagedInMem(const void *ptr);
	inline void MarkStackPush(const void *ptr);
	inline const void* MarkStackPop();
	inline bool MarkStackEmpty();


	void**				m_Managed;
	int					m_ManLast;
	unsigned int		m_ManSize;

	drgIntAddr			m_HeapStart;
	drgIntAddr			m_HeapEnd;
	unsigned int		m_VolumeThreshold;

	gc_typerec*			m_Types;
	int					m_TypesLast;
	int					m_TypesSize;

	const void**		m_RootList;
	int					m_RootsLast;
	int					m_RootsSize;

	const void**		m_MarkingStack;
	bool				m_MarkingStackReset;
	int					m_StackSize;
	int					m_StackLast;
	int					m_MarkingType;
	const void*			m_MarkingObject;
	bool				m_StackOverflowed;
	bool				m_StackOverflowed2;

	drgMemBlock			m_GCSpace;
	notify_func*		m_ClientNotify;
	bool				m_DebugEnabled;
	bool				m_CollectRequested;
	bool				m_CollectInProgress;
	bool				m_CollectRestart;
	bool				m_MarkInProgress;
	bool				m_ManIsCompact;
	bool				m_GCdisabled;
	int					m_NumCollects;
	int					m_CollectStage;
	unsigned int		m_VolAllocs;
	unsigned int		m_NumAllocs;
	unsigned int		m_TotalMalloc;
	unsigned int		m_CollectAllocs;
	unsigned int		m_CollectMin;
	unsigned int		m_CollectFrame;
    
/*#if defined(_IPHONE)
	static drgGarbageColector*	m_pInstance;
#else*/
	static drgGarbageColector	m_Instance;
//#endif
};


inline void drgGarbageColector::CheckInfo()
{
	m_GCSpace.CheckInfo();
}

inline void* drgGarbageColector::AllocInternal(unsigned int size)
{
	++m_NumAllocs;
	void* retval = m_GCSpace.Reserve(size, GC_MIN_HUNKSIZE MEM_INFO_AUTO);
	drgMemory::MemSet( retval, 0, size );
	return retval;
}

inline void* drgGarbageColector::Seal(const char *ptr)
{
	ptr = (const char*)(GC_CLRPTR(ptr));
	ptr += GC_MIN_HUNKSIZE;
	return (void*)ptr;
}

inline void* drgGarbageColector::UnSeal(const char *ptr)
{
	ptr = (const char*)(GC_CLRPTR(ptr));
	ptr -= GC_MIN_HUNKSIZE;
	return (void*)ptr;
}

inline int drgGarbageColector::FindManaged(const void *ptr)
{
	if( (GC_PPTR(ptr) < m_HeapStart) || (GC_PPTR(ptr) > m_HeapEnd) )
		return -1;

	unsigned int mgidx = GC_INFO_I(ptr);
	if(mgidx > (unsigned int)m_ManLast)
		return -1;

	if (GC_CLRPTR(m_Managed[mgidx]) == ptr)
		return mgidx;

	return -1;
}

inline int drgGarbageColector::FindManagedInMem(const void *ptr)
{
	if( (GC_PPTR(ptr) < m_HeapStart) || (GC_PPTR(ptr) > m_HeapEnd) )
		return -1;

	unsigned int mgidx = GC_INFO_I(ptr);
	if(mgidx > (unsigned int)m_ManLast)
		return -1;

	if ( (ptr >= GC_CLRPTR(m_Managed[mgidx])) && (GC_PPTR(ptr) <= (GC_PPTR(GC_CLRPTR(m_Managed[mgidx]))+GC_INFO_S(m_Managed[mgidx]))) )
		return mgidx;

	return -1;
}

inline void drgGarbageColector::MarkStackPush(const void *ptr)
{
	MarkLive(ptr);

	m_StackLast++;
	if (m_StackLast == m_StackSize)
	{
		m_StackOverflowed = true;
		m_StackLast--;
	}
	else
	{
		m_MarkingStack[m_StackLast] = ptr;
	}
}

inline const void* drgGarbageColector::MarkStackPop()
{
	return ((m_StackLast<0) ? NULL : m_MarkingStack[m_StackLast--]);
}

inline bool drgGarbageColector::MarkStackEmpty()
{
	return m_StackLast==-1;
}

inline bool drgGarbageColector::IsManaged(const void *ptr)
{
	if (!GC_ADDRESS_VALID(ptr))
		return false;
	else
		return FindManaged(ptr) != -1;
}

inline int drgGarbageColector::GetType(const void *ptr)
{
	int i = FindManaged(ptr);
	return GC_INFO_T(m_Managed[i]);
}

inline unsigned int drgGarbageColector::GetSize(const void *ptr)
{
	int i = FindManaged(ptr);
	return GC_INFO_S(m_Managed[i]);
}


extern void drgGCInitExt(unsigned int size, notify_func* func, bool debug);
extern void drgGCDeInitExt();
extern void* drgGCAllocExt(unsigned int size, unsigned int frame, bool refs);
extern void drgGCFreeExt(void* ptr);
extern bool drgGCIdleCountExt(unsigned int frame, int count);
extern int drgGCCollectNowExt(unsigned int frame);
extern void drgEnableExt();
extern bool drgDisableExt( bool block );
extern void drgGCRootExt(void* ptr);
extern void drgGCUnRootExt(void* ptr);
extern int drgGCRegTypeExt(char* desc, int flags, clear_func clrf, finalize_func* finalf);
extern bool drgGCSetTypeExt(void* ptr, int type);
extern void drgGCSetDataExt(void* ptr, void* data);
extern unsigned int drgGCUsedAmountExt();
extern clear_func* drgGetClearExt(int type);
extern void drgPrintInfoExt();


#endif // __DRG_GCOLLECT_H__
