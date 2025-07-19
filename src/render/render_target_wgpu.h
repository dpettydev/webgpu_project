#ifndef _DRG_RENDER_TARGET_WGPU_H_
#define _DRG_RENDER_TARGET_WGPU_H_

#include "system/global.h"

#if USE_WEBGPU
#include "render/texture.h"

struct drgRenderTargetPlatformDataWGPU
{
	unsigned int	m_TargetHandle;
};

typedef drgRenderTargetPlatformDataWGPU drgRenderTargetPlatformData;

#endif  // USE_WEBGPU

#endif  // _DRG_RENDER_TARGET_WGPU_H_