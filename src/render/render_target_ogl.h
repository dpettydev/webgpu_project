#ifndef _DRG_RENDER_TARGET_OGL_H_
#define _DRG_RENDER_TARGET_OGL_H_

#include "system/global.h"

#if USE_OPEN_GL
#include "render/texture.h"

struct drgRenderTargetPlatformDataOGL
{
	unsigned int	m_TargetHandle;
};

typedef drgRenderTargetPlatformDataOGL drgRenderTargetPlatformData;

#endif  // USE_OPEN_GL

#endif  // _DRG_RENDER_TARGET_OGL_H_