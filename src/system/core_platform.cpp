

#include "system/extern_inc.h"
#include "core/memmgr.h"
#include "system/engine.h"
#include "system/file.h"
#include "core/file/file_manager.h"
#include "core/system.h"
#include "core/cmdarg.h"
#include "system/engine_messages.h"

#if USE_MEM_MANAGER
drgMemBlock g_MainBlocks(DRG_MEM_MAIN);
#endif
void ANDROID_CallGC();

void InitMemory()
{
#if USE_MEM_MANAGER
#if defined(EMSCRIPTEN)
	g_MainBlocks.Set((10 * 1024 * 1024), 0);
#elif defined(_ANDROID)
	g_MainBlocks.Set((224 * 1024 * 1024), 0);
#elif defined(_IPHONE)
	g_MainBlocks.Set((224 * 1024 * 1024), 0);
#else
	g_MainBlocks.Set((512 * 1024 * 1024), 0);
#endif
#endif
}

#if defined(_LINUX)
drgOSMutex drgPlatMutexCreate()
{
	return (drgOSMutex)0x3EAD;
}

void drgPlatMutexLock(drgOSMutex &mutex)
{
}

bool drgPlatMutexLockTry(drgOSMutex &mutex)
{
	return true;
}

void drgPlatMutexUnlock(drgOSMutex &mutex)
{
}
#elif defined(_IPHONE)
void drgPlatMutexCreate(drgOSMutex *mutex)
{
	// if( mutex->value != 0)
	//     return;
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	// pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	int result = pthread_mutex_init(mutex, &attr);
	if (result != 0)
		drgPrintWarning("Failed to initialize mutex: %d", result);
}

void drgPlatMutexDestroy(drgOSMutex *mutex)
{
	if (mutex->__sig == 0)
		return;
	pthread_mutex_destroy(mutex);
	// mutex->value = 0;
}

void drgPlatMutexLock(drgOSMutex &mutex)
{
	if (mutex.__sig == 0)
		return;
	int result = pthread_mutex_lock(&mutex);
	if (result != 0)
	{
		printf("Failed to lock mutex: %d", result);
		*(int *)0 = 0;
	}
}

bool drgPlatMutexLockTry(drgOSMutex &mutex)
{
	if (mutex.__sig == 0)
		return false;
	return (pthread_mutex_trylock(&mutex) == 0);
}

void drgPlatMutexUnlock(drgOSMutex &mutex)
{
	if (mutex.__sig == 0)
		return;
	int result = pthread_mutex_unlock(&mutex);
	if (result != 0)
	{
		drgPrintError("Mutex failed to unlock: %d", result);
		*(int *)0 = 0;
	}
}
#elif defined(_ANDROID)
void drgPlatMutexCreate(drgOSMutex *mutex)
{
	pthread_mutex_t *mutexlock = (pthread_mutex_t *)mutex;

	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	// pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	int result = pthread_mutex_init(mutexlock, &attr);
	if (result != 0)
		drgPrintWarning("Failed to initialize mutex: %d", result);
}

void drgPlatMutexDestroy(drgOSMutex *mutex)
{
	pthread_mutex_destroy((pthread_mutex_t *)mutex);
	mutex->value = 0;
}

void drgPlatMutexLock(drgOSMutex &mutex)
{
	int result = pthread_mutex_lock((pthread_mutex_t *)&mutex);
	if (result != 0)
	{
		char buffer[128];
		sprintf(buffer, "Failed to lock mutex: %d", result);
		__android_log_write(ANDROID_LOG_INFO, "ACE", buffer);
		*(int *)0 = 0;
	}
}

bool drgPlatMutexLockTry(drgOSMutex &mutex)
{
	return (pthread_mutex_trylock((pthread_mutex_t *)&mutex) == 0);
}

void drgPlatMutexUnlock(drgOSMutex &mutex)
{
	int result = pthread_mutex_unlock((pthread_mutex_t *)&mutex);
	if (result != 0)
	{
		drgPrintError("Mutex failed to unlock: %d", result);
		*(int *)0 = 0;
	}
}
#elif defined(EMSCRIPTEN)
void drgPlatMutexCreate(drgOSMutex *mutex)
{
	mutex->value = 32;
}

void drgPlatMutexDestroy(drgOSMutex *mutex)
{
	mutex->value = 0;
}

void drgPlatMutexLock(drgOSMutex &mutex)
{
	while (mutex.value != 32)
	{
	}
	mutex.value = 54;
}

bool drgPlatMutexLockTry(drgOSMutex &mutex)
{
	if (mutex.value == 32)
	{
		mutex.value = 54;
		return true;
	}
	return false;
}

void drgPlatMutexUnlock(drgOSMutex &mutex)
{
	if (mutex.value == 54)
		mutex.value = 32;
}

#elif defined(_WINDOWS)
void drgPlatMutexCreate(drgOSMutex *mutex)
{
	drgOSMutex ret = CreateSemaphore(NULL, 1, 1, NULL);
	assert(ret);
	(*mutex) = ret;
}

void drgPlatMutexDestroy(drgOSMutex *mutex)
{
	/*CloseHandle( m_Mutex );*/
	(*mutex) = NULL;
}

void drgPlatMutexLock(drgOSMutex &mutex)
{
	WaitForSingleObject(mutex, INFINITE);
}

bool drgPlatMutexLockTry(drgOSMutex &mutex)
{
	return ((WAIT_TIMEOUT == WaitForSingleObject(mutex, 0)) ? false : true);
}

void drgPlatMutexUnlock(drgOSMutex &mutex)
{
	ReleaseSemaphore(mutex, 1, NULL);
}
#endif

unsigned int drgGetTime()
{
#if defined(_3DS)
	return (unsigned int)nn::fnd::DateTime::GetNow().GetMilliSecond();
#else
	return (unsigned int)time(NULL);
#endif
}

#if defined(EMSCRIPTEN)
#include <emscripten.h>
#include <html5.h>
bool USE_SCRIPT_INTERFACE = false;

extern "C" void drgLibAppInterfaceInit();
extern "C" void drgLibAppInterfaceCleanUp();
extern "C" void drgLibAppInterfaceLoad(const char *szFileName);
extern "C" void drgLibAppInterfaceUpdate();

int em_close()
{
	emscripten_cancel_main_loop();
	int ret = drgSystem::CleanUp();
	g_MainBlocks.CleanUp();
	return ret;
}

void em_update()
{
	if (drgEngine::IsClosing())
	{
		em_close();
		return;
	}
	drgSystem::Update();
	drgSystem::Draw();
	return;
}

extern "C" void em_init()
{
	int ret = drgSystem::Init();
	if (ret != 0)
	{
		drgPrintOut("Fail system init!");
		emscripten_cancel_main_loop();
		return;
	}
}

int main(int argc, char *argv[])
{
	InitVStringBlocks(48 * 1024);
	drgCommandArg::InitList(argc, argv);
	drgFileManager::SetResourceFolder("C:/ace/runtime/");
	drgFileManager::SetCacheFolder("C:/ace/runtime/cache/");
	int ret = drgSystem::Init();
	if (ret != 0)
	{
		drgPrintOut("Fail system init!");
		return -1;
	}
	emscripten_set_main_loop(em_update, 60, 1);
	return 0;
}

void AppInterface::Init()
{
	drgLibAppInterfaceInit();
}

void AppInterface::CleanUp()
{
	drgLibAppInterfaceCleanUp();
}

void AppInterface::Load(const char *szFileName)
{
	drgLibAppInterfaceLoad(szFileName);
}

void AppInterface::Update()
{
	drgLibAppInterfaceUpdate();
}

void AppInterface::ProcessMessage(int type, char *value)
{
}

#elif (defined(_ANDROID) || defined(_IPHONE))

#else

int main(int argc, char *argv[])
{
	InitVStringBlocks(64 * 1024);
	drgCommandArg::InitList(argc, argv);
	drgFileManager::SetResourceFolder("C:/TEMP/webgpu_project/runtime/");
	drgFileManager::SetCacheFolder("C:/TEMP/webgpu_project/runtime/cache/");

	int ret = drgSystem::Init();
	if (ret != 0)
		return ret;
	while (!drgEngine::IsClosing())
	{
		drgSystem::Update();
		drgSystem::Draw();
	}
	ret = drgSystem::CleanUp();
#if USE_MEM_MANAGER
	g_MainBlocks.CleanUp();
#endif
	return ret;
}

void AppInterface::Init()
{
	if (AppInterfaceNative::g_NativeAppInterfaceInit != NULL)
	{
		AppInterfaceNative::g_NativeAppInterfaceInit();
	}
}

void AppInterface::CleanUp()
{
	if (AppInterfaceNative::g_NativeAppInterfaceCleanUp != NULL)
	{
		AppInterfaceNative::g_NativeAppInterfaceCleanUp();
	}
}

void AppInterface::Load(const char *szFileName)
{
}

void AppInterface::Update(double dtime)
{
	if (AppInterfaceNative::g_NativeAppInterfaceUpdate != NULL)
	{
		AppInterfaceNative::g_NativeAppInterfaceUpdate(dtime);
	}
}

void AppInterface::ProcessMessage(int type, char *value)
{
}

#endif

#if defined(_LLVM)
// #define DEFAULT_SECURITY_COOKIE 0xBB40E64E
// void __attribute__((fastcall)) __chkstk(unsigned int* szLocalFrame) asm ("__chkstk");
//
// unsigned int* __security_cookie = (unsigned int*)DEFAULT_SECURITY_COOKIE;
//
// void __attribute__((fastcall)) __security_check_cookie(unsigned int* cookie)
//{
//	int test = 0;
// }
//
// void __attribute__((fastcall)) __chkstk(unsigned int* szLocalFrame)
//{
//	int test = 0;
// }
//
// extern "C" unsigned long long _aulldiv( unsigned long long a, unsigned long long b )
//{
//	return (a / b);
// }
//
// extern "C" unsigned long long _aullrem( unsigned long long a, unsigned long long b )
//{
//     return (a % b);
// }
//
// extern "C" long long _alldiv( long long a, long long b )
//{
//	return (a / b);
// }
//
// extern "C" long long _allrem( long long a, long long b )
//{
//     return (a % b);
// }

extern "C" void *malloc_pnet(unsigned int size)
{
	return NULL;
}

extern "C" void *realloc_pnet(void *mem, unsigned int size)
{
	return NULL;
}

extern "C" void free_pnet(void *mem)
{
}

// extern "C" int atexit(void (__cdecl *)(void))
//{
//	return 0;
// }
//
// extern "C" void __cxa_atexit()
//{
// }
//
// extern "C" void __dso_handle()
//{
// }

#endif

#if (defined(_ANDROID))

// adb logcat -s "ACE", "DEBUG", "Chartboost", "AmazonInsightsSDK"
// RETAIL_BUILD;

#include <jni.h>
#include <EGL/egl.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <poll.h>
#include <pthread.h>
#include <sched.h>

AAssetManager *g_JavaAssetMgr = NULL;
JavaVM *g_pJavaVM = NULL;
extern float g_AndroidWindowWidth;
extern float g_AndroidWindowHeight;
extern int g_AndroidFormat;
extern float g_XDPI;
extern float g_YDPI;
extern bool g_IsPausing;
extern bool g_IsPaused;
extern ANativeWindow *g_NativeWindow;
extern EGLDisplay g_Display;
extern EGLSurface g_Surface;
extern EGLContext g_Context;

extern void OnAndroidTouchDown(float x, float y, int pointerIndex, float pressure);
extern void OnAndroidTouchUp(float x, float y, int pointerIndex);
extern void OnAndroidTouchMove(float x, float y, int pointerIndex);
bool g_EngineIsInitialized = false;
bool g_ReleaseResources = false;

extern "C"
{
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_init(JNIEnv *env, jobject obj, jint width, jint height, jint format, jstring cache);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_run(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_step(JNIEnv *env, jobject obj, jfloat fElapsedTime);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_createAssetManager(JNIEnv *env, jobject obj, jobject assetManager);
	JNIEXPORT bool JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onTouchMove(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint pointerIndex);
	JNIEXPORT bool JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onTouchDown(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint pointerIndex, jfloat pressure);
	JNIEXPORT bool JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onTouchUp(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint pointerIndex, jfloat pressure);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onPause(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onResume(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onStart(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onStop(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onFocusLost(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onFocusGained(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onRestart(JNIEnv *env, jobject);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onBackPressed(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onCreate(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onDestroy(JNIEnv *env, jobject obj);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_setSurface(JNIEnv *env, jclass cls, jobject surfaceObject);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_clearSurface(JNIEnv *env, jclass cls);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_setDPI(JNIEnv *env, jobject obj, jfloat xDPI, jfloat yDPI);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onAdDone(JNIEnv *env, jobject obj, jint type, jint result);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onThirdPartyLoginDone(JNIEnv *env, jobject obj, jint type, jstring result);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onMerchantSkuDetailsReceived(JNIEnv *env, jobject obj, jint type, jstring result);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onMerchantSkuPurchaseComplete(JNIEnv *env, jobject obj, jint type, jstring result);
	JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onABTestingValuesReceived(JNIEnv *env, jobject obj);
}

static int engine_init_display()
{
	enum
	{
		WINDOW_FORMAT_RGBA_8888 = 1,
		WINDOW_FORMAT_RGBX_8888 = 2,
		WINDOW_FORMAT_RGB_565 = 4,
	};
	const EGLint attribs[] = {
		EGL_LEVEL, 0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_RED_SIZE, EGL_DONT_CARE,
		EGL_GREEN_SIZE, EGL_DONT_CARE,
		EGL_BLUE_SIZE, EGL_DONT_CARE,
		EGL_NONE};

	EGLint ai32ContextAttribs[] =
		{
			EGL_CONTEXT_CLIENT_VERSION, 2,
			EGL_NONE};

	EGLBoolean ret_val;
	EGLint major, minor;
	EGLint w, h;
	EGLint format = 2;
	EGLint numConfigs;
	EGLConfig *matchingConfigs = NULL;
	EGLConfig config = NULL;
	EGLSurface surface = NULL;
	EGLContext context = NULL;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	ret_val = eglInitialize(display, &major, &minor);
	drgPrintOut("eglInitialize: %s(major:%i, minor:%i)  0x%x\r\n", (ret_val == EGL_TRUE) ? "TRUE" : "FALSE", major, minor, display);

	drgPrintOut("g_AndroidFormat: %i\r\n", g_AndroidFormat);
	int inc565 = 50;
	int inc888 = 50;
	if (g_AndroidFormat == WINDOW_FORMAT_RGB_565)
		inc565 += 10;
	else
		inc888 += 10;

	ret_val = eglChooseConfig(display, attribs, NULL, 0, &numConfigs);
	drgPrintOut("eglChooseConfig %i: %s\r\n", numConfigs, (ret_val == EGL_TRUE) ? "TRUE" : "FALSE");
	if (numConfigs > 0)
	{
		int bestmatch = 0;
		int totalmatch = 0;
		EGLint red, green, blue, depth, native, type, surftype, level, cid, sample;
		matchingConfigs = (EGLConfig *)drgMemAlloc(numConfigs * sizeof(EGLConfig));
		ret_val = eglChooseConfig(display, attribs, matchingConfigs, numConfigs, &numConfigs);
		for (int ii = 0; ii < numConfigs; ii++)
		{
			totalmatch = 0;

			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_RED_SIZE, &red);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_BLUE_SIZE, &blue);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_GREEN_SIZE, &green);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_DEPTH_SIZE, &depth);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_NATIVE_RENDERABLE, &native);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_RENDERABLE_TYPE, &type);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_SURFACE_TYPE, &surftype);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_LEVEL, &level);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_CONFIG_ID, &cid);
			eglGetConfigAttrib(display, matchingConfigs[ii], EGL_SAMPLES, &sample);

#if 0 //! defined(RETAIL_BUILD)
			drgPrintOut("\r\nCONFIG\r\n  EGL_CONFIG_ID:  %i\r\n");
			drgPrintOut("  EGL_CONFIG_ID:  %i\r\n", cid);
			drgPrintOut("  EGL_LEVEL:      %i\r\n", level);
			drgPrintOut("  EGL_RED_SIZE:   %i\r\n", red);
			drgPrintOut("  EGL_GREEN_SIZE: %i\r\n", green);
			drgPrintOut("  EGL_BLUE_SIZE:  %i\r\n", blue);
			drgPrintOut("  EGL_DEPTH_SIZE: %i\r\n", depth);
			drgPrintOut("  EGL_SAMPLES:    %i\r\n", sample);
			drgPrintOut("  EGL_NATIVE_RENDERABLE: %i\r\n", native);
			drgPrintOut("  EGL_RENDERABLE_TYPE:   0x%x\r\n", type);
			drgPrintOut("  EGL_SURFACE_TYPE:      0x%x\r\n\r\n", surftype);
#endif

			if (config == NULL)
				config = matchingConfigs[ii];

			if ((red == 5) && (green == 6) && (blue == 5))
				totalmatch += inc565;
			if ((red == 8) && (green == 8) && (blue == 8))
				totalmatch += inc888;
			if (depth == 0)
				totalmatch += 5;
			if (sample == 0)
				totalmatch += 5;
			if (native == EGL_FALSE)
				totalmatch += 10;

			if (bestmatch < totalmatch)
			{
				bestmatch = totalmatch;
				config = matchingConfigs[ii];
			}
		}
		drgMemFree(matchingConfigs);
	}

	ret_val = eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
	drgPrintOut("eglGetConfigAttrib Format %i: %s\r\n", format, (ret_val == EGL_TRUE) ? "TRUE" : "FALSE");

	int set_buf_ret = ANativeWindow_setBuffersGeometry(g_NativeWindow, 0, 0, format);
	drgPrintOut("ANativeWindow_setBuffersGeometry: %i\r\n", set_buf_ret);

	surface = eglCreateWindowSurface(display, config, g_NativeWindow, NULL);
	drgPrintOut("eglCreateWindowSurface: 0x%x\r\n", surface);

	context = eglCreateContext(display, config, EGL_NO_CONTEXT, ai32ContextAttribs);
	drgPrintOut("eglCreateContext: 0x%\r\nx", context);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		drgPrintOut("Unable to eglMakeCurrent\r\n");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	drgPrintOut("eglQuerySurface EGL_WIDTH: %s\r\n", (ret_val == EGL_TRUE) ? "TRUE" : "FALSE");
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);
	drgPrintOut("eglQuerySurface EGL_HEIGHT: %s\r\n", (ret_val == EGL_TRUE) ? "TRUE" : "FALSE");

	g_Display = display;
	g_Context = context;
	g_Surface = surface;

	drgEngine::LoadingScreenStart();

	return 0;
}

jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	g_pJavaVM = vm;
	return JNI_VERSION_1_6;
}

void JNI_OnUnload(JavaVM *vm, void *reserved)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "JNI_OnUnload");
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_init(JNIEnv *env, jobject obj, jint width, jint height, jint format, jstring cache)
{
	// drgCommandArg::InitList(argc, argv);
	g_AndroidWindowWidth = (float)width;
	g_AndroidWindowHeight = (float)height;
	g_AndroidFormat = (int)format;

	jboolean isCopy;
	const char *cache_dir = env->GetStringUTFChars(cache, &isCopy);
	drgFileManager::SetResourceFolder("ace/runtime/");
	drgFileManager::SetCacheFolder(cache_dir);
	drgPrintOut("Set Cache Folder: %s\n", cache_dir);
	if (isCopy == JNI_TRUE)
		env->ReleaseStringUTFChars(cache, cache_dir);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_run(JNIEnv *env, jobject obj)
{
	InitVStringBlocks(48 * 1024);
	engine_init_display();
	int ret = drgSystem::Init();
	g_EngineIsInitialized = true;

	while (!drgEngine::IsClosing())
	{
		if (g_Display == NULL)
		{
			if (g_ReleaseResources == true)
			{
				drgEngine::ReleaseResources();
				g_ReleaseResources = false;
			}

			if ((g_IsPaused == false) && (g_NativeWindow != NULL))
			{
				drgPrintOut("Re Init Display!\n");
				engine_init_display();
				drgEngine::RecreateResources();
				drgEngine::LoadingScreenEnd();
			}
		}
		else
		{
			if (g_EngineIsInitialized && !drgEngine::IsClosing() && g_IsPaused == false)
			{
				drgSystem::Update();
				drgSystem::Draw();
			}
		}
		drgEngine::ProcessMessages();

		if (drgEngine::IsClosing())
		{
			int ret = drgSystem::CleanUp();
#if USE_MEM_MANAGER
			g_MainBlocks.CleanUp();
#endif
			exit(0);
		}
	}
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_step(JNIEnv *env, jobject obj, jfloat fElapsedTime)
{
	// if( g_EngineIsInitialized && !drgEngine::IsClosing() && g_IsPaused == false )
	//{
	//	drgSystem::Update();
	//	drgSystem::Draw();
	// }

	// if( drgEngine::IsClosing() )
	//{
	//	int ret = drgSystem::CleanUp();
	//	g_MainBlocks.CleanUp();
	//	exit(0);
	// }
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_createAssetManager(JNIEnv *env, jobject obj, jobject assetManager)
{
	g_JavaAssetMgr = AAssetManager_fromJava(env, assetManager);
	assert(g_JavaAssetMgr);
}

JNIEXPORT bool JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onTouchDown(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint pointerIndex, jfloat pressure)
{
	OnAndroidTouchDown((float)x, (float)y, (int)pointerIndex, (float)pressure);
}

JNIEXPORT bool JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onTouchMove(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint pointerIndex)
{
	OnAndroidTouchMove((float)x, (float)y, (int)pointerIndex);
}

JNIEXPORT bool JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onTouchUp(JNIEnv *env, jobject obj, jfloat x, jfloat y, jint pointerIndex, jfloat pressure)
{
	OnAndroidTouchUp((float)x, (float)y, (int)pointerIndex);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onPause(JNIEnv *env, jobject obj)
{
	g_IsPausing = true;
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on pause");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONPAUSE, NULL, false);
	drgEngine::AddEngineMessage(record);
	while (!g_IsPaused)
		drgThread::SleepCurrent(1);
	g_IsPausing = false;
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onResume(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on resume");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONRESUME, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onStart(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on start");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSTART, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onStop(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on stop");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSTOP, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onFocusLost(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on focus lost");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONFOCUS_LOST, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onFocusGained(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on focus gained");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONFOCUS_GAINED, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onRestart(JNIEnv *env, jobject)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on restart gained");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONRESTART, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onBackPressed(JNIEnv *env, jobject)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on back button pressed");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONBACK_PRESSED, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onCreate(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on create");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONCREATE, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onDestroy(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI on destroy");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONDESTROY, NULL, false);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_setSurface(JNIEnv *env, jclass cls, jobject surfaceObject)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] Surface has been set");
	g_NativeWindow = ANativeWindow_fromSurface(env, surfaceObject);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_clearSurface(JNIEnv *env, jclass cls)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] Surface has been cleared");
	g_ReleaseResources = true;
	g_NativeWindow = NULL;
	g_Display = NULL;
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_setDPI(JNIEnv *env, jobject obj, jfloat xDPI, jfloat yDPI)
{
	char buffer[128];
	sprintf(buffer, "[ENGINE] DPI has been set X DPI %f, Y DPI %f", xDPI, yDPI);
	__android_log_write(ANDROID_LOG_INFO, "ACE", buffer);

	g_XDPI = xDPI;
	g_YDPI = yDPI;
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onAdDone(JNIEnv *env, jobject obj, jint type, jint result)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI ad callback");
	char *returned_res = (char *)drgMemAlloc(16);
	if (result == 0)
		drgString::Copy(returned_res, "false");
	else
		drgString::Copy(returned_res, "true");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONAD_DONE, returned_res, true);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onThirdPartyLoginDone(JNIEnv *env, jobject obj, jint type, jstring result)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI third party login callback");
	char *returned_id = (char *)malloc(256);
	jboolean isCopy;
	const char *str_result = env->GetStringUTFChars(result, &isCopy);
	strcpy(returned_id, str_result);
	if (isCopy == JNI_TRUE)
		env->ReleaseStringUTFChars(result, str_result);

	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONTHIRDPARTYLOGIN_DONE, returned_id, true);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onMerchantSkuDetailsReceived(JNIEnv *env, jobject obj, jint type, jstring result)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI merchant sku details callback");
	char *skudetail = (char *)malloc(4096);
	jboolean isCopy;
	const char *str_result = env->GetStringUTFChars(result, &isCopy);
	strcpy(skudetail, str_result);
	if (isCopy == JNI_TRUE)
		env->ReleaseStringUTFChars(result, str_result);

	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSKUDETAILSRECEIVED, skudetail, true);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onMerchantSkuPurchaseComplete(JNIEnv *env, jobject obj, jint type, jstring result)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI merchant sku purchase results callback");
	char *saledetail = (char *)malloc(1024);
	jboolean isCopy;
	const char *str_result = env->GetStringUTFChars(result, &isCopy);
	strcpy(saledetail, str_result);
	if (isCopy == JNI_TRUE)
		env->ReleaseStringUTFChars(result, str_result);

	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSKUPURCHASERESULT, saledetail, true);
	drgEngine::AddEngineMessage(record);
}

JNIEXPORT void JNICALL Java_com_zenjoyintl_slotfantasy_AceLib_onABTestingValuesReceived(JNIEnv *env, jobject obj)
{
	__android_log_write(ANDROID_LOG_INFO, "ACE", "[ENGINE] JNI ab testing values received callback");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONABTESTINGVALUESRECEIVED, NULL, false);
	drgEngine::AddEngineMessage(record);
}

// Java call interfaces
void ANDROID_GetAndroidID(char *android_id)
{
	drgString::Copy(android_id, "DEADCODE");
	drgDebug::Print("Getting Android ID");
	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetAndroidID", "()Ljava/lang/String;");
	if (mid_GetAndroidID)
	{
		jboolean isCopy;
		jstring result = (jstring)env->CallStaticObjectMethod(class_AceEngine, mid_GetAndroidID);
		const char *ptr = env->GetStringUTFChars(result, &isCopy);
		drgString::Copy(android_id, ptr);
		if (isCopy == JNI_TRUE)
			env->ReleaseStringUTFChars(result, ptr);
	}
	env->DeleteLocalRef(class_AceEngine);
}

int ANDROID_GetAdState(int type)
{
	drgDebug::Print("Getting Ad State");
	int retval = 0;
	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetAdState", "(I)I");
	if (mid_GetAndroidID)
	{
		jint ret_obj = env->CallStaticIntMethod(class_AceEngine, mid_GetAndroidID, type);
		retval = (jint)ret_obj;
	}
	env->DeleteLocalRef(class_AceEngine);

	ANDROID_CallGC();
	return retval;
}

void ANDROID_CallGC()
{
	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "CallGC", "()V");
	if (mid_GetAndroidID)
	{
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID);
	}
	env->DeleteLocalRef(class_AceEngine);
}

int ANDROID_ShowAd(int type)
{
	int retval = 0;
	drgDebug::Print("Showing Ad");

	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "ShowAd", "(I)I");
	if (mid_GetAndroidID)
	{
		jint ret_obj = env->CallStaticIntMethod(class_AceEngine, mid_GetAndroidID, type);
		retval = (jint)ret_obj;
	}
	env->DeleteLocalRef(class_AceEngine);
	return retval;
}

void ANDROID_OpenExternalUrl(char *url)
{
	drgDebug::Print("Opening external URL");

	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "OpenExternalUrl", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_url = env->NewStringUTF(url);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_url);
		env->DeleteLocalRef(st_url);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_OpenExternalEmail(char *addr, char *subject, char *text)
{
	JNIEnv *env;
	drgDebug::Print("Opening external email");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "OpenExternalEmail", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_addr = env->NewStringUTF(addr);
		jstring st_subject = env->NewStringUTF(subject);
		jstring st_text = env->NewStringUTF(text);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_addr, st_subject, st_text);
		env->DeleteLocalRef(st_addr);
		env->DeleteLocalRef(st_subject);
		env->DeleteLocalRef(st_text);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_OpenAppRating()
{
	JNIEnv *env;
	drgDebug::Print("Opening app rating");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "OpenAppRating", "()V");
	if (mid_GetAndroidID)
	{
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID);
	}
	env->DeleteLocalRef(class_AceEngine);
}

long long ANDROID_ShowThirdPartyLogin(int type)
{
	drgDebug::Print("Showing third party login");

	long long retval = 0;
	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "ShowThirdPartyLogin", "(I)J");
	if (mid_GetAndroidID)
	{
		jobject ret_obj = env->CallStaticObjectMethod(class_AceEngine, mid_GetAndroidID, type);
		retval = (jint)ret_obj;
		env->DeleteLocalRef(ret_obj);
	}
	env->DeleteLocalRef(class_AceEngine);
	return retval;
}

int ANDROID_GetThirdPartyLoginReady(int type)
{
	int retval = 0;
	drgDebug::Print("Getting Third Party Login Ready");

	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetThirdPartyLoginReady", "(I)I");
	if (mid_GetAndroidID)
	{
		jint ret_obj = env->CallStaticIntMethod(class_AceEngine, mid_GetAndroidID, type);
		retval = (jint)ret_obj;
	}
	env->DeleteLocalRef(class_AceEngine);
	return retval;
}

long long ANDROID_GetThirdPartyLoginId(int type)
{
	drgDebug::Print("Getting Third Party Login Id");

	long long retval = 0;
	JNIEnv *env;
	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetThirdPartyLoginId", "(I)J");
	if (mid_GetAndroidID)
	{
		jobject ret_obj = env->CallStaticObjectMethod(class_AceEngine, mid_GetAndroidID, type);
		retval = (jint)ret_obj;
		env->DeleteLocalRef(ret_obj);
	}
	env->DeleteLocalRef(class_AceEngine);
	return retval;
}

void ANDROID_GetMerchantSkuData(int type, char *skulist)
{
	JNIEnv *env;
	drgDebug::Print("Getting Merchant SKU data");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetMerchantSkuData", "(ILjava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_skulist = env->NewStringUTF(skulist);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, type, st_skulist);
		env->DeleteLocalRef(st_skulist);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_PurchaseMerchantSku(int type, char *sku)
{
	JNIEnv *env;
	drgDebug::Print("Purchase Merchant SKU data");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "PurchaseMerchantSku", "(ILjava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_skulist = env->NewStringUTF(sku);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, type, st_skulist);
		env->DeleteLocalRef(st_skulist);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_GetABTestingProjects(char *projects)
{
	JNIEnv *env;
	drgDebug::Print("Getting AB testing projects");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceInsight");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetABTestingProjects", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_projects = env->NewStringUTF(projects);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_projects);
		env->DeleteLocalRef(st_projects);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_GetABTestingProjectValues(char *project, char *values, char *outvalues)
{
	JNIEnv *env;
	drgDebug::Print("Getting AB testing project values");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceInsight");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetABTestingProjectValues", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");
	if (mid_GetAndroidID)
	{
		jboolean isCopy;
		jstring st_project = env->NewStringUTF(project);
		jstring st_values = env->NewStringUTF(values);
		jstring result = (jstring)env->CallStaticObjectMethod(class_AceEngine, mid_GetAndroidID, st_project, st_values);
		const char *ptr = env->GetStringUTFChars(result, &isCopy);
		drgString::Copy(outvalues, ptr);
		if (isCopy == JNI_TRUE)
			env->ReleaseStringUTFChars(result, ptr);
		env->DeleteLocalRef(st_project);
		env->DeleteLocalRef(st_values);
	}

	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_TriggerInsightEvent(char *eventname)
{
	JNIEnv *env;
	drgDebug::Print("Triggering insight event");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceInsight");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "TriggerEvent", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_event = env->NewStringUTF(eventname);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_event);
		env->DeleteLocalRef(st_event);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_TriggerInsightEvent(char *eventname, char *attributes, char *metrics)
{
	JNIEnv *env;
	drgDebug::Print("Triggering insight event");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceInsight");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "TriggerEvent", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_event = env->NewStringUTF(eventname);
		jstring st_attrs = env->NewStringUTF(attributes);
		jstring st_metrics = env->NewStringUTF(metrics);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_event, st_attrs, st_metrics);
		env->DeleteLocalRef(st_event);
		env->DeleteLocalRef(st_attrs);
		env->DeleteLocalRef(st_metrics);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_SubmitQueuedInsightEvents()
{
	JNIEnv *env;
	drgDebug::Print("Submit Queued insight events");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceInsight");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "SubmitQueuedABTestingEvents", "()V");
	if (mid_GetAndroidID)
	{
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_GetPushNotificationToken(char *token)
{
	ANDROID_CallGC();

	JNIEnv *env;
	drgDebug::Print("Get Push Notification Token");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "GetPushNotificationToken", "()Ljava/lang/String;");
	if (mid_GetAndroidID)
	{
		jboolean isCopy;
		jstring result = (jstring)env->CallStaticObjectMethod(class_AceEngine, mid_GetAndroidID);
		const char *ptr = env->GetStringUTFChars(result, &isCopy);
		drgString::Copy(token, ptr);
		if (isCopy == JNI_TRUE)
			env->ReleaseStringUTFChars(result, ptr);
	}

	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_EnableScreenTimeout(bool enable)
{
	JNIEnv *env;
	drgDebug::Print("Enable screen timeout");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jobject ret_obj = NULL;
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "EnableScreenTimeout", "(I)V");
	if (mid_GetAndroidID)
	{
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, (enable) ? -1 : 0);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_AnalyticsInit(const char *tracking_id)
{
	JNIEnv *env;
	drgDebug::Print("Analytics Init");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "AnalyticsInit", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_tracking_id = env->NewStringUTF(tracking_id);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_tracking_id);
		env->DeleteLocalRef(st_tracking_id);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_AnalyticsSetUserID(const char *user_id)
{
	JNIEnv *env;
	drgDebug::Print("Analytics Set UserID");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "AnalyticsSetUserID", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_user_id = env->NewStringUTF(user_id);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_user_id);
		env->DeleteLocalRef(st_user_id);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_AnalyticsHitScreen(const char *screen_name)
{
	JNIEnv *env;
	drgDebug::Print("Analytics Hit Screen");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "AnalyticsHitScreen", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_screen_name = env->NewStringUTF(screen_name);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_screen_name);
		env->DeleteLocalRef(st_screen_name);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_AnalyticsHitEvent(const char *category, const char *action, const char *label)
{
	JNIEnv *env;
	drgDebug::Print("Analytics Hit Event");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "AnalyticsHitEvent", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_category = env->NewStringUTF(category);
		jstring st_action = env->NewStringUTF(action);
		jstring st_label = env->NewStringUTF(label);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_category, st_action, st_label);
		env->DeleteLocalRef(st_category);
		env->DeleteLocalRef(st_action);
		env->DeleteLocalRef(st_label);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_AnalyticsHitEventVal(const char *category, const char *action, const char *label, unsigned int value)
{
	JNIEnv *env;
	drgDebug::Print("Analytics Hit Event");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "AnalyticsHitEvent", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;I)V");
	if (mid_GetAndroidID)
	{
		jstring st_category = env->NewStringUTF(category);
		jstring st_action = env->NewStringUTF(action);
		jstring st_label = env->NewStringUTF(label);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_category, st_action, st_label, value);
		env->DeleteLocalRef(st_category);
		env->DeleteLocalRef(st_action);
		env->DeleteLocalRef(st_label);
	}
	env->DeleteLocalRef(class_AceEngine);
}

void ANDROID_AnalyticsHitException(const char *description)
{
	JNIEnv *env;
	drgDebug::Print("Analytics Hit Exception");

	g_pJavaVM->AttachCurrentThread(&env, NULL);
	jclass class_AceEngine = NULL;
	jmethodID mid_GetAndroidID = NULL;
	class_AceEngine = env->FindClass("com/zenjoyintl/slotfantasy/AceEngine");
	mid_GetAndroidID = env->GetStaticMethodID(class_AceEngine, "AnalyticsHitException", "(Ljava/lang/String;)V");
	if (mid_GetAndroidID)
	{
		jstring st_description = env->NewStringUTF(description);
		env->CallStaticVoidMethod(class_AceEngine, mid_GetAndroidID, st_description);
		env->DeleteLocalRef(st_description);
	}
	env->DeleteLocalRef(class_AceEngine);
}

#endif //_ANDROID

#if defined(_IPHONE)

/*#include <jni.h>
#include <EGL/egl.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>*/
#include <poll.h>
#include <pthread.h>
#include <sched.h>

/*AAssetManager* g_JavaAssetMgr = NULL;
JavaVM* g_pJavaVM = NULL;*/
extern float g_IOSWindowWidth;
extern float g_IOSWindowHeight;
extern float g_XDPI;
extern float g_YDPI;
extern bool g_IsPausing;
extern bool g_IsPaused;
/*extern ANativeWindow* g_NativeWindow;
extern EGLDisplay g_Display;
extern EGLSurface g_Surface;
extern EGLContext g_Context;*/

class AceEngine;
extern AceEngine *g_pAceEngine;

extern void OnAndroidTouchDown(float x, float y, int pointerIndex, float pressure);
extern void OnAndroidTouchUp(float x, float y, int pointerIndex);
extern void OnAndroidTouchMove(float x, float y, int pointerIndex);
bool g_EngineIsInitialized = false;

static int engine_init_display()
{
	/*const EGLint attribs[] =
	{
		EGL_LEVEL, 0,
		EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
		EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
		EGL_NATIVE_RENDERABLE, EGL_FALSE,
		EGL_DEPTH_SIZE, EGL_DONT_CARE,
		EGL_BLUE_SIZE, 8,
		EGL_GREEN_SIZE, 8,
		EGL_RED_SIZE, 8,
		EGL_NONE
	};
	EGLint w, h, dummy, format;
	EGLint numConfigs;
	EGLConfig config;
	EGLSurface surface;
	EGLContext context;

	EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

	eglInitialize(display, 0, 0);

	eglChooseConfig(display, attribs, &config, 1, &numConfigs);

	eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);

	ANativeWindow_setBuffersGeometry(g_NativeWindow, 0, 0, format);

	EGLint ai32ContextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	surface = eglCreateWindowSurface(display, config, g_NativeWindow, NULL);
	context = eglCreateContext(display, config, EGL_NO_CONTEXT, ai32ContextAttribs);

	if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE)
	{
		drgPrintOut("Unable to eglMakeCurrent");
		return -1;
	}

	eglQuerySurface(display, surface, EGL_WIDTH, &w);
	eglQuerySurface(display, surface, EGL_HEIGHT, &h);

	g_Display = display;
	g_Context = context;
	g_Surface = surface;*/

	drgEngine::LoadingScreenStart();

	return 0;
}

extern char strHomePath[];

void IOS_Init(int width, int height, int ppi, char *home_path, char *resource_dir, char *cache_dir)
{
	g_IOSWindowWidth = (float)width;
	g_IOSWindowHeight = (float)height;

	g_XDPI = ppi;
	g_YDPI = ppi;

	strcpy(strHomePath, home_path);
	drgFileManager::SetResourceFolder(resource_dir);
	drgFileManager::SetCacheFolder(cache_dir);
	drgPrintOut("Set Cache Folder: %s\n", cache_dir);
}

void IOS_Update()
{
	if (!g_EngineIsInitialized)
	{
		InitVStringBlocks(48 * 1024);
		engine_init_display();
		int ret = drgSystem::Init();
		g_EngineIsInitialized = true;
	}
	drgSystem::Update();
	/*if (drgEngine::IsClosing())
	{
		g_EngineIsInitialized = false;
		int ret = drgSystem::CleanUp();
		//g_MainBlocks.CleanUp();
		exit(0);
	}*/
}

void IOS_Draw()
{
	if (g_EngineIsInitialized)
		drgSystem::Draw();

	// drgEngine::ProcessMessages();
}

void IOS_CallGC()
{
}

char returned_res[16];
void IOS_CALLBACK_onAdDone(int type, int result)
{
	drgDebug::Print("[ENGINE] JNI ad callback\r\n");
	if (result == 0)
		strcpy(returned_res, "false");
	else
		strcpy(returned_res, "true");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONAD_DONE, returned_res, false);
	drgEngine::AddEngineMessage(record);
}

void IOS_CALLBACK_onABTestingValuesReceived()
{
	drgDebug::Print("[ENGINE] JNI ab testing values received callback\r\n");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONABTESTINGVALUESRECEIVED, NULL, false);
	drgEngine::AddEngineMessage(record);
}

void IOS_CALLBACK_onMerchantSkuDetailsReceived(char *details)
{
	drgDebug::Print("[ENGINE] JNI merchant sku details received callback\r\n");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSKUDETAILSRECEIVED, details, false);
	drgEngine::AddEngineMessage(record);
}

void IOS_CALLBACK_onMerchantSkuPurchaseComplete(char *result)
{
	drgDebug::Print("[ENGINE] JNI merchant sku purchase results callback\r\n");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONSKUPURCHASERESULT, result, false);
	drgEngine::AddEngineMessage(record);
}

void IOS_CALLBACK_onThirdPartyLoginDone(char *result)
{
	drgDebug::Print("[ENGINE] JNI third party login callback");
	drgMessageRecord *record = new drgMessageRecord(DRG_ENGINE_MESSAGE_ONTHIRDPARTYLOGIN_DONE, result, false);
	drgEngine::AddEngineMessage(record);
}

#endif //_IPHONE
