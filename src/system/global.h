#ifndef __DRG_GLOBAL_H__
#define __DRG_GLOBAL_H__

#include "core/memmgr.h"
#include "util/color.h"
#include "system/debug.h"

#define USE_WEBGPU 0
#define USE_OPEN_GL 1

#ifdef _IPHONE
#define DRG_MAX_PATH (350) // massive paths possible on iOS
#else
#define DRG_MAX_PATH (260)
#endif
#define DRG_MEMALIGN_16
#define WATER_DIM (32)

// Standard constants
#undef FALSE
#undef TRUE
#undef NULL

#define FALSE 0
#define TRUE 1
#define NULL 0

#define DRG_NO_FLAGS 0

#define DRG_SHRT_MIN (-32768)
#define DRG_SHRT_MAX (32767)
#define DRG_USHRT_MAX (0xffff)
#define DRG_INT_MIN (-2147483647 - 1)
#define DRG_INT_MAX (2147483647)
#define DRG_UINT_MAX (0xffffffff)

#define VALIDPTR(x) 1
#define ALIGNED(x, bits) (!(((int)(x)) & ((1 << bits) - 1)))

#define DRG_MAX_LEVEL_DETAIL (3)

#define LIGHT_TYPE_DIRECTIONAL (0)
#define LIGHT_TYPE_OMNI (1)
#define LIGHT_TYPE_SPOT (2)

#define DRG_MAX_MOPRH_OFFSET (3)

#define DRG_FOUR_CHAR(x) (((unsigned char)x[0]) | ((unsigned char)x[1] << 8) | ((unsigned char)x[2] << 16) | (unsigned char)x[3] << 24)
#define DRG_TWO_CHAR(x) ((unsigned char)x[0] | (unsigned char)x[1] << 8)

#define INT64_FMT "lld"
#define UINT64_FMT "llu"

#ifdef _VERBOSE
#define VERBOSE(x) x
#else
#define VERBOSE(x)
#endif

#if defined(_MSC_FULL_VER) && (_MSC_FULL_VER >= 140050320)
#define DRG_DEPRECATE __declspec(deprecated("This has been deprecated and will be removed in a future release"))
#elif defined(_MSC_VER) && _MSC_VER >= 1300 && !defined(MIDL_PASS)
#define DRG_DEPRECATE __declspec(deprecated)
#elif defined(__GNUC__) && (__GNUC__ > 3 || (__GNUC__ == 3 && (__GNUC_MINOR__ > 1 || (__GNUC_MINOR__ == 1 && __GNUC_PATCHLEVEL__ >= 1))))
#define DRG_DEPRECATE __attribute__((deprecated))
#else
#define DRG_DEPRECATE
#endif

#if defined(EMSCRIPTEN)
#include "system/global_emscript.h"
#elif defined _WINDOWS
#include "system/global_win.h"
#else
#include "system/global_gen.h"
#endif

extern unsigned int drgGetTime();

extern bool USE_SCRIPT_INTERFACE;

#include "util/profile.h"

#endif // __DRG_GLOBAL_H__
