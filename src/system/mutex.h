#ifndef __DRG_MUTEX_H__
#define __DRG_MUTEX_H__


#if defined(_ANDROID) || defined(EMSCRIPTEN)
struct drgOSMutex
{
    int volatile value;
};
#elif defined(_IPHONE)
#include <thread>
typedef pthread_mutex_t drgOSMutex;
#else
typedef void* drgOSMutex;
#endif

extern void drgPlatMutexCreate(drgOSMutex* mutex);
extern void drgPlatMutexDestroy(drgOSMutex* mutex);
extern void drgPlatMutexLock(drgOSMutex& mutex);
extern bool drgPlatMutexLockTry(drgOSMutex& mutex);
extern void drgPlatMutexUnlock(drgOSMutex& mutex);

class drgMutex
{
public:
	inline drgMutex()
	{
		drgPlatMutexCreate(&m_Mutex);
	}

	inline ~drgMutex()
	{
		drgPlatMutexDestroy(&m_Mutex);
	}

	inline void Lock()
	{
		drgPlatMutexLock( m_Mutex );
	}

	inline void Unlock()
	{
		drgPlatMutexUnlock( m_Mutex );
	}

	inline bool TryLock()
	{
		return drgPlatMutexLockTry( m_Mutex );
	}

private:

	drgOSMutex		m_Mutex;
};


// MutexLock(mu) acquires mu when constructed and releases it when destroyed.
class drgMutexLock
{
public:
	explicit drgMutexLock(drgMutex *mu) : m_Mu(mu) { m_Mu->Lock(); };
	~drgMutexLock()
	{
		m_Mu->Unlock();
	}

	void ManualUnlock()
	{
		m_Mu->Unlock();
	}

private:
	drgMutex* const m_Mu;
};

#endif // __DRG_MUTEX_H__

