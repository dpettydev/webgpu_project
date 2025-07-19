#ifndef __DRG_DEBUG_H__
#define __DRG_DEBUG_H__


#if defined(RETAIL_BUILD)
#define RETAIL_IN(x)				x
#define RETAIL_OUT(x)
#else
#define RETAIL_IN(x)
#define RETAIL_OUT(x)				x
#endif

#define DRG_DEBUG_NETWORK (0)

#ifdef assert
#undef assert
#endif


class drgDebug
{
public:
	static void Init();
#ifdef RETAIL_BUILD
	static inline void Print(const char* buff, int number=0) {};
	static inline void Notify(const char *fmt, ... ) {};
	static inline void Error(const char *fmt, ... ) {};
	static inline void CopyToContext( char *fmt, ... ) {};
#else
	static void Print(const char *buff, int num=0);
	static void Notify(const char *fmt, ... );
	static void Error(const char *fmt, ... );
	static void CopyToContext( char *fmt, ... );
#endif

protected:
	drgDebug();
	~drgDebug();

	static void DisplayMsg(char *buff, int num=0);
};

class drgDebugContext
{
public:
#ifdef RETAIL_BUILD
	drgDebugContext( char *fmt, ... ) {};
	~drgDebugContext() {};
#else
	drgDebugContext( char *fmt, ... );
	~drgDebugContext();
#endif

protected:
	drgDebugContext() {};

	char*	m_OldContext;
};

#ifdef RETAIL_BUILD
inline void drgPrintError( const char *fmt, ... ) {};
inline void drgPrintWarning( const char *fmt, ... ) {};
inline void drgPrintOut( const char *fmt, ... ) {};
inline void drgPrintF(int num, const char *fmt, ... ) {};
#else
extern void drgPrintError( const char *fmt, ... );
extern void drgPrintWarning( const char *fmt, ... );
extern "C" void drgPrintOut( const char *fmt, ... );
extern "C" void drgPrintF(int num, const char *fmt, ... );
#endif


#ifdef _DEBUG
//	#define except(exp)				((exp) ? (void)0 : _except_drg(__FILE__, __LINE__, #exp))
	#define except(exp, exec, errtype)	((exp) ? (void)0 : _except_drg(__FILE__, __LINE__, exec, #exp, errtype))
	#define throwexcept(exp, exec, errtype)	_except_drg(__FILE__, __LINE__, exec, exp, errtype)
	extern void _assert_drg(const char *file, unsigned int line, const char *assertion);
	#define assert(exp)				((exp) ? (void)0 : _assert_drg(__FILE__, __LINE__, #exp))
	#define assertmsg(exp, msg)		((exp) ? (void)0 : _assert_drg(__FILE__, __LINE__, msg))
	#define DRG_CONTEXT_FUNC(name)	drgDebugContext FUNCCONTEXT("%s", #name);
	#define DB_ONLY(x)				x
#else
	#define assert(exp)
	#define assertmsg(exp, msg)
	//extern void _assert_drg(const char *file, unsigned int line, const char *assertion);
	//#define assert(exp)				((exp) ? (void)0 : _assert_drg(__FILE__, __LINE__, #exp))
	//#define assertmsg(exp, msg)		((exp) ? (void)0 : _assert_drg(__FILE__, __LINE__, msg))
	#define DRG_CONTEXT_FUNC(name)
	#define DB_ONLY(x)
//	#define except(exp)				((exp) ? (void)0 : _except_drg(__FILE__, __LINE__, #exp))
	#define except(exp, exec, errtype)	((exp) ? (void)0 : _except_drg(__FILE__, __LINE__, exec, #exp, errtype))
	#define throwexcept(exp, exec, errtype)	_except_drg(__FILE__, __LINE__, exec, #exp, errtype)
#endif  // _DEBUG
	 
enum DRG_DEBUG_FAIL_TYPE
{
	DRG_DEBUG_FAIL_NONE = 0,
	DRG_DEBUG_FAIL_FDS = 1111,
	DRG_DEBUG_FAIL_ASSETS,
	DRG_DEBUG_FAIL_CONNECT,
	DRG_DEBUG_FAIL_USER_START
};

enum DRG_DEBUG_LEVEL
{
	DRG_DEBUG_LEVEL_DEFAULT = 0,
	DRG_DEBUG_LEVEL_WARNING,
	DRG_DEBUG_LEVEL_ERROR,
	DRG_DEBUG_LEVEL_EXCEPTION,
	DRG_DEBUG_LEVEL_ENUM_SIZE
};

extern DRG_DEBUG_FAIL_TYPE drgDebugGetFailType();
extern void drgDebugReSetFailType();
extern void drgDebugSetFailType( DRG_DEBUG_FAIL_TYPE type );

#endif // __DRG_DEBUG_H__

