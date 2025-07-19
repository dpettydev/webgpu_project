
#include "system/global.h"

#if defined(_WINDOWS)
#define FORCEINLINE inline
#define WINVER 0x0501
#define _WIN32_WINNT 0x0501
#define _GDI32_
#include <winsock2.h>
#include <Windows.h>
#include <direct.h>
#include <WinBase.h>
#endif
#include <time.h>
#include <errno.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#else // __EMSCRIPTEN__
#if USE_OPEN_GL
#include "extern/glad/glad.h"
#endif
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif // __EMSCRIPTEN__

#if USE_WEBGPU
#include <webgpu/webgpu.h>
#ifdef __EMSCRIPTEN__
#include <emscripten/html5_webgpu.h>
#endif // __EMSCRIPTEN__

class drgRenderContext_WEBGPU;

#define WGPU_ERROR( message )  {  } 
//#define WGPU_ERROR( message )  { GLenum error = glGetError(); if( error != GL_NO_ERROR ){ HandleGLError( error, message ); } } 
extern void HandleWGPUError(int error, const char* message);
extern void SetContextWGPU(drgRenderContext_WEBGPU* context);
extern drgRenderContext_WEBGPU* GetContextWGPU();

#endif // USE_WEBGPU

#if USE_OPEN_GL
#define GL_ERROR(message)                  \
    {                                      \
        GLenum error = glGetError();       \
        if (error != GL_NO_ERROR)          \
        {                                  \
            HandleGLError(error, message); \
        }                                  \
    }
extern void HandleGLError(GLenum error, const char *message);
#endif